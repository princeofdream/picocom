#!/bin/bash

config_platform="$(uname -o)"
config_platform=${config_platform#*/}
config_platform_arch=""
config_generator=""
config_toolchain=""
config_build_type="Release"

config_tier1=""
config_jobs=$(cat /proc/cpuinfo|grep "^processor"|tail -1|awk -F":" '{print $2}')
if [[ "$config_jobs" == "" ]]; then
    config_jobs=3
elif [[ ${config_jobs} -le 1 ]]; then
    config_jobs=1
fi

config_build_mode="all"
config_clean_build="false"
config_demo_apps="default"
config_intercomm=""

cmd_readlink="readlink -f"
######### end setup env to get script path ############

script_path=$(dirname "$($cmd_readlink "$0")")
# config_top_path=$(pwd)
config_top_path=${script_path}
if [[ "$config_top_path" == "" ]]; then
    config_top_path=$(pwd)
fi
config_ppcom_output_path=""
config_core_path="${config_top_path}"
config_exit="false"
config_cmake_param=""
config_build_top=""
config_build_path=""
config_build_output=""
config_install_top=""
config_outpath=""
config_projects_list=""
config_start_build_time=$(date +"%Y-%m-%d_%H:%M:%S")

######### start setup basic script env ############
config_debug=0
config_log_count=0
config_log_color="true"
config_log_dir="${config_top_path}"
config_log_name="log_"$(basename $0)".log"
config_log_path=""
######### End Setup basic script env ############
log_common ()
{
	local var_log_common_type="$1"
	local var_log_common_current_time="$(date +%H:%M:%S)"
	local var_log_common_fg_color=31
	local var_log_common_bg_color=32
	local var_log_common_label="log"

	case ${var_log_common_type} in
		debug | dbg )
			var_log_common_fg_color=31;
			var_log_common_bg_color=34;
			var_log_common_label="dbg";
			if [[ $config_debug == 0 ]]; then
				return 0;
			fi
			shift;
			;;
		normal | nor | log )
			var_log_common_fg_color=31;
			var_log_common_bg_color=32;
			var_log_common_label="log";
			shift;
			;;
		warning | wrn )
			var_log_common_fg_color=31;
			var_log_common_bg_color=33;
			var_log_common_label="wrn";
			shift;
			;;
		error | err )
			var_log_common_fg_color=31;
			var_log_common_bg_color=31;
			var_log_common_label="err";
			shift;
			;;
	esac

	if [[ "$config_log_color" == "true" ]]; then
		echo -e "[0;${var_log_common_fg_color};1m[ ${var_log_common_current_time} ] [${var_log_common_label}]\t[0m[0;${var_log_common_bg_color};1m$* [0m"
	else
		echo -e "[ ${var_log_common_current_time} ] [${var_log_common_label}]\t$* "
	fi

	if [[ $config_log_path"x" != "x" ]]; then
		if [[ $config_log_count == 0 ]]; then
			echo -e "[ ${var_log_common_current_time} ] [${var_log_common_label}]\t$* " > $config_log_path
		else
			echo -e "[ ${var_log_common_current_time} ] [${var_log_common_label}]\t$* " >> $config_log_path
		fi
	fi
	config_log_count=$((config_log_count+1))
	return 0
}	# ----------  end of function logd  ----------

logd ()
{
	log_common "dbg" "$@"
}	# ----------  end of function loge  ----------

log ()
{
	log_common "log" "$@"
}	# ----------  end of function loge  ----------

logw ()
{
	log_common "wrn" "$@"
}	# ----------  end of function loge  ----------

loge ()
{
	log_common "err" "$@"
}	# ----------  end of function loge  ----------


usage() {
cat <<USAGE

Usage:
    bash $0 [OPTIONS]

Description:
    Sample script

OPTIONS:
    -t, --target
        Target [ qnx | linux | windows ]

Usage:
    $0

USAGE
}

long_opts="help,target:,clean"
long_opts+=""

getopt_cmd=$(getopt -o ht:c --long "$long_opts" \
            -n $(basename $0) -- "$@") || \
            { echo -e "\nERROR: Getopt failed. Extra args\n"; usage; exit 1;}

eval set -- "$getopt_cmd"

while true; do
    case "$1" in
        -h|--help) usage; exit 0;;
        -c|--clean) config_clean_build="true";;
        -t|--target) config_platform="$2"; shift;;
        --) shift; break;;
    esac
    shift
done

if [[ "${config_platform}" == "" ]]; then
    config_platform=$1
fi
if [[ "${config_tier1}" == "" ]]; then
    config_tier1=$2
fi

## name from cmake CMAKE_SYSTEM_NAME

build_clean ()
{
    if [[ "${config_clean_build}" != "true" ]]; then
        return 0
    fi

    if [[ -e ${config_top_path}/output || -h ${config_top_path}/output ]]; then
        rm -rf ${config_top_path}/output
    fi
    return 0
}	# ----------  end of function build_clean  ----------

remove_exist_output ()
{
    rm -rf output
    return 0
}	# ----------  end of function remove_exist_output  ----------

install_release ()
{
    return 0
}	# ----------  end of function install_release  ----------

setup_common_cmake_param ()
{
    config_cmake_param="-DCMAKE_INSTALL_PREFIX=${config_install_top}"
    return 0;
}	# ----------  end of function setup_common_cmake_param  ----------

build_target ()
{
    local var_ret
    local var_target_top=""


    if [[ ! -d "${config_build_top}" ]]; then
        mkdir -p ${config_build_top}
    fi

    cd ${config_build_top} || return 127

    setup_common_cmake_param

    var_target_top=${config_top_path}

    cd ${config_build_top}
    log "cmake ${var_target_top} ${config_cmake_param}"
    cmake ${var_target_top} ${config_cmake_param}
    log "cmake --build . --config ${config_build_type} --target install -j${config_jobs}"
    cmake --build . --config ${config_build_type} --target install -j${config_jobs}
    var_ret=$?
    if [[ ${var_ret} -ne 0 ]]; then
        loge "ERROR: Cmake Install Failed!!"
        return ${var_ret}
    fi

    return ${var_ret}
}	# ----------  end of function build_target  ----------

main_func ()
{
    config_build_top=${config_top_path}/output/build
    config_install_top=${config_top_path}/output/release

    build_clean
    if [[ ! -e ${config_build_top} ]]; then
        if [[ -h ${config_build_top} ]]; then
            rm -rf ${config_build_top}
        fi
        mkdir -p ${config_build_top}
    fi

    build_target

    return 0;
}	# ----------  end of function main_func  ----------


main_func $*
main_ret=$?
if [[ $main_ret -ne 0 ]]; then
    loge "Build ppcom Failed with error $main_ret."
else
    log "Build ppcom Success."
fi
log "from [ ${config_start_build_time} ] to [ $(date +"%Y-%m-%d_%H:%M:%S") ]"
exit $main_ret



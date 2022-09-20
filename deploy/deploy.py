#!/usr/bin/python3
import os
import sys
import pathlib
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--required', help='Install required libs', action='store_true')
parser.add_argument('--scripts', help='Install build scripts', action='store_true')
parser.add_argument('--libs', help='Install build project libs', action='store_true')
parser.add_argument('--apps', help='Install build project apps', action='store_true')
parser.add_argument('--full', help='Do a full install', action='store_true')
args = parser.parse_args()

if len(sys.argv) == 1:
   parser.print_help()
   sys.exit("\nPlease provide program arguments")

bin_install_dir = "/usr/local/bin"
config_dir = os.path.expanduser("~") + "/.infonet/configs"
pathlib.Path(config_dir).mkdir(parents=True, exist_ok=True)

requirements = [
   "git",
   "wget",
   "build-essential",
   "cmake",
   "pkg-config",
   "liblua5.3-dev",
   "librocksdb-dev",
   "libsqlite3-dev",
   "libcurl4-openssl-dev"
]

pre_build_scripts = [
   "scripts/install_cpputest.sh"
]

project_libs = {
   "libutil":"git@github.com:infonet-dev/libutil.git",
   "nettle":"git@github.com:infonet-dev/nettle.git",
   "tomlplusplus":"git@github.com:infonet-dev/tomlplusplus.git",
   "cpp-httplib":"git@github.com:infonet-dev/cpp-httplib.git",
   "cpp-sqlitelib":"git@github.com:infonet-dev/cpp-sqlitelib.git",
   "crate":"git@github.com:infonet-dev/crate.git",
}

project_apps = {
   "monolith":"git@github.com:infonet-dev/monolith.git",
}

working_dir = os.getcwd()

def check_for_root_access():
   if os.geteuid() != 0:
      sys.exit("Need to be root")

def check_result(code):
   error_code = code >> 8
   if error_code != 0:
      sys.exit("Failed to install execute")

def install_system_requirements():
   for item in requirements:
      check_result(os.system("sudo apt install -y " + item))

def run_pre_build_scripts():
   for item in pre_build_scripts:
      check_result(os.system("bash " + item))

def install_project_libs():
   pathlib.Path("build").mkdir(parents=True, exist_ok=True)
   os.chdir("build")
   for name, item in project_libs.items():
      check_result(os.system("git clone " + item))
      os.chdir(name)
      pathlib.Path("build").mkdir(parents=True, exist_ok=True)
      os.chdir("build")
      check_result(os.system("cmake -DCMAKE_BUILD_TYPE=Release ../"))
      check_result(os.system("sudo make install"))
      os.chdir("../../")
   os.chdir("../")

def install_apps():
   pathlib.Path("build").mkdir(parents=True, exist_ok=True)
   os.chdir("build")
   for name, item in project_apps.items():
      check_result(os.system("git clone " + item))
      os.chdir(name)
      pathlib.Path("build").mkdir(parents=True, exist_ok=True)
      os.chdir("build")
      check_result(os.system("cmake -DCMAKE_BUILD_TYPE=Release ../"))
      check_result(os.system("make -j5"))
      check_result(os.system("sudo cp " + name + " " + bin_install_dir))
      check_result(os.system("cp configs/* " + config_dir))
      os.chdir("../../")

if (args.full):
   check_for_root_access()
   install_system_requirements()
   run_pre_build_scripts()
   install_project_libs()
   install_apps()

if (args.required):
   check_for_root_access()
   install_system_requirements()

if (args.scripts):
   check_for_root_access()
   run_pre_build_scripts()

if (args.libs):
   install_project_libs()

if (args.apps):
   install_apps()

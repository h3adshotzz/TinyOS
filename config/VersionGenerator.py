#
# VersionGenerator Python Script
#
# This script is used for generating version strings in the following format:
#
#   project-aaaa.bb.cc.R
#
# The version number is derrived from an existing MasterVersion file, with
# each component as follows:
# 
#   aaaa  -   source version number, written as x.y.z
#   bb    -   major build number
#   cc    -   minor build number
#   R     -   revision
#
# On each build, the minor build number (cc) is incremented by 1 until it
# reaches 99, after which the major build number (bb) is incremented by 1,
# cc reset to 0 and the incrementing continues.
#
# The revision (R) is set manually, as well as the source version number
# (aaaa). Overall, a project has three version numbers:
#
#   Project Version Number    -   Traditional X.Y.Z version, manually set.
#   Source Version Number     -   Traditional X.Y.Z version, manually set
#                                 for the source tree.
#   Build Number              -   Auto-incrementing bb.cc formatted version
#                                 updated on each build.
#
#

from jinja2 import Environment, FileSystemLoader
from dataclasses import dataclass
import jinja2 as j2
import argparse


@dataclass
class Version:
  data: list
  source: int
  build_maj: int
  build_min: int
  build_rev: int


def parse_master_version_file(file, buildtype, project):

  with open (file, "r") as f:
    data = f.readlines()

  # build version: aaaa.bb.cc.R
  build_major = int(data[1].strip().split(".")[0])
  build_minor = int(data[2].strip().split(".")[0])
  build_rev = int(data[3].strip().split(".")[0])

  source_version = data[0].strip().split(".")
  source_version = "{}{}{}".format(source_version[0], source_version[1], source_version[2])

  # create version object
  vers = Version(data, source_version, build_major, build_minor, build_rev)
  return vers


def update_source_number(project, version):
  if version.build_min >= 99:
    version.build_min = 0
    version.build_maj += 1
  else:
    version.build_min += 1

  SOURCE_VERSION = f"{project}-{version.source}.{version.build_maj}.{version.build_min}.{version.build_rev}"
  print(SOURCE_VERSION)

def write_version_files(f_mv, f_hdr, version, build_type, project, template):
  # master version file
  version.data[1] = f"{version.build_maj}"
  version.data[2] = f"{version.build_min}"
  with open(f_mv, "w") as f:
    f.writelines(version.data)

  # version header
  env = j2.Environment(loader=FileSystemLoader(template))
  template = env.get_template("version.h.j2")

  SOURCE_VERSION = f"{project}-{version.source}.{version.build_maj}.{version.build_min}.{version.build_rev}"
  header = template.render(
    

# main
if __name__ == "__main__":

  #
  # Parameters:
  #
  #   -m, --masterversion     MasterVersion text file with described version
  #                           numbers.
  #   -t, --template          Jinja2 template for the resulting header file.
  #   -b, --build-type        RELEASE, DEBUG, INTERNAL
  #   -p, --project           Project name
  #
  #
  parser = argparse.ArgumentParser()
  parser.add_argument("-m", "--master", action="store")
  parser.add_argument("-t", "--template", action="store")
  parser.add_argument("-b", "--build-type", action="store")
  parser.add_argument("-p", "--project", action="store")
  parser.add_argument("-o", "--output", action="store")
  args = parser.parse_args()

  if args.master:
    parsed_version = parse_master_version_file(args.master, args.build_type, args.project)
    new_version = update_source_number(args.project, parsed_version)

# Copyright 2021 The gRPC Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM fedora:40

RUN yum clean all && yum update -y && yum install -y python3 python3-pip
RUN python3 -m pip install virtualenv

RUN yum groupinstall -y "Development Tools"
RUN yum install -y redhat-rpm-config
RUN yum install -y gcc-c++
RUN yum install -y python3-devel

RUN yum install -y findutils

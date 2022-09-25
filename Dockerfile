FROM archlinux:latest
LABEL Description="Build environment"

ENV HOME /root

SHELL ["/bin/bash", "-c"]

RUN pacman -Syu --noconfirm && pacman -S --noconfirm clang cmake qt6-base vulkan-devel glm gtest make

COPY DockerEntrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
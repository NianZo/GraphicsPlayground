FROM archlinux:latest
LABEL Description="Build environment"

ENV HOME /root

SHELL ["/bin/bash", "-c"]

RUN pacman -Syu --noconfirm && pacman -S --noconfirm clang cmake qt6-base vulkan-devel glm gtest make git xorg-server vulkan-icd-loader mesa vulkan-swrast
#RUN echo LIBGL_ALWAYS_SOFTWARE=true >> /etc/environment
#RUN echo GALLIUM_DRIVER=llvmpipe >> /etc/environment

COPY DockerEntrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
FROM archlinux:latest
LABEL Description="Build environment"

ENV HOME /root

SHELL ["/bin/bash", "-c"]

RUN pacman -Syu --noconfirm && pacman -S --noconfirm clang cmake qt6-base vulkan-devel glm gtest make git xorg-server xorg-xinit vulkan-icd-loader mesa vulkan-swrast xorg-server-xvfb
#RUN echo LIBGL_ALWAYS_SOFTWARE=true >> /etc/environment
#RUN echo GALLIUM_DRIVER=llvmpipe >> /etc/environment

COPY DockerEntrypoint.sh /entrypoint.sh
#COPY 10-headless.conf /etc/X11/xorg.conf.d/10-headless.conf

ENTRYPOINT ["/entrypoint.sh"]
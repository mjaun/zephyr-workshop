FROM ghcr.io/zephyrproject-rtos/zephyr-build:v0.29.2

# Additional tools (udev is a dependency for the serial monitor VS code extension)
RUN sudo apt update \
 && sudo apt install -y udev less bash-completion \
 && sudo apt clean

# Configure bash completion
RUN mkdir -p $HOME/.bash_completion.d \
 && echo 'source <(cd /workdir && west completion bash)' > $HOME/.bash_completion.d/west-completion.bash

# Install PyOCD package for STM32U3
RUN pyocd pack install STM32U3

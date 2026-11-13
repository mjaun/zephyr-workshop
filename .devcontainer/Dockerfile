FROM ghcr.io/zephyrproject-rtos/zephyr-build:v0.29.2

# Additional tools (udev is a dependency for the serial monitor VS code extension)
RUN sudo apt update \
 && sudo apt install -y curl udev less bash-completion \
 && sudo apt clean

# Configure bash completion
RUN mkdir -p $HOME/.bash_completion.d \
 && echo 'source <(cd /workdir && west completion bash)' > $HOME/.bash_completion.d/west-completion.bash

# Install PyOCD package for STM32U3
RUN pyocd pack install STM32U3

# Pre-release cortex-debug VS Code extension (the released version has a bug that breaks debugging).
# Installed by path from devcontainer.json.
ARG CORTEX_DEBUG_VERSION=1.13.0-pre8
RUN sudo mkdir -p /opt/vscode-extensions \
 && sudo curl -fsSL -o /opt/vscode-extensions/cortex-debug.vsix \
      https://github.com/Marus/cortex-debug/releases/download/v${CORTEX_DEBUG_VERSION}/cortex-debug-${CORTEX_DEBUG_VERSION}.vsix \
 && echo "a7ef7aa063d3845274dd917d87f8f81db281b3b40d1af7e27a0f05e510b2144f  /opt/vscode-extensions/cortex-debug.vsix" | sha256sum -c -

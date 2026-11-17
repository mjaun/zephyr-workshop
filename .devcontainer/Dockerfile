FROM ghcr.io/zephyrproject-rtos/zephyr-build:v0.29.2

# Additional tools (udev is a dependency for the serial monitor VS code extension)
RUN sudo apt update \
 && sudo apt install -y curl udev less bash-completion \
 && sudo apt clean

# Configure bash completion
RUN mkdir -p $HOME/.bash_completion.d \
 && echo 'source <(cd /workdir && west completion bash)' > $HOME/.bash_completion.d/west-completion.bash

# PyOCD device pack for STM32U3. PYOCD_PROJECT_DIR makes pyocd load the pyocd.yaml
# that points to the pack, for every pyocd invocation (west flash, cortex-debug).
COPY STM32U3xx_DFP.pack /opt/pyocd-packs/STM32U3xx_DFP.pack
RUN echo "pack: /opt/pyocd-packs/STM32U3xx_DFP.pack" | sudo tee /opt/pyocd-packs/pyocd.yaml
ENV PYOCD_PROJECT_DIR=/opt/pyocd-packs

# Pre-release cortex-debug VS Code extension (the released version has a bug that breaks debugging).
# Version 1.13.0-pre8, from https://github.com/Marus/cortex-debug/releases.
COPY cortex-debug.vsix /opt/vscode-extensions/cortex-debug.vsix

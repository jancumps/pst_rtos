# pico_scpi_usbtmc_labtool
LabVIEW compatible instrument on a Raspberry Pico
FreeRTOS SMP version


Prerequisites:  
Toolchain (and IDE) that can build Pico projects with CMake  
Pico C SDK 1.5  
FreeRTOS Kernel

Clone the repository with its subrepositories, to get the application, the Pico SCPI USBTMC_LabLib (PSL) sources, and the Jan Breuer SCPI library:  
git clone https://github.com/jancumps/pst_rtos --recurse-submodules  

Add this variable to your environment:  

    PICO_SDK_PATH (e.g.: C:/Users/jancu/Documents/Pico/pico-sdk)  
    FREERTOS_KERNEL_PATH (e.g.: C:/Users/jancu/Documents/git/git_freertos/FreeRTOS/FreeRTOS/Source)

If you use VSCode, you can define them via Preferences -> User -> Extensions -> CMake Tools -> CMake: Configure Environment.  


[documentation](https://github.com/jancumps/pico_scpi_usbtmc_labtool/wiki) on GitHub wiki

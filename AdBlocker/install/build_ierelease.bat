::input parameters
::prod, test, dev

@echo off
cls

:: SET BUILD PARAMETERS!!!!!
set version=0.8.5
set release=64
set comment=Test Release 0.8.5

set pathVisualStudio=C:\Programmer\Microsoft Visual Studio 9.0\Common7\Tools
set pathAdvancedInstaller=C:\Programmer\Caphyon\Advanced Installer 7.2.1


:: Write version to config files
echo #define IEPLUGIN_VERSION "%version%" > ..\source\Shared\Version.h

echo BUILD %version%

if %1 == setvar  (
  echo SET path to visual studio
  echo ******************
  "%pathVisualStudio%\vsvars32.bat"
  goto end
)

if %1 == prod  (
  echo CREATE PRODUCTION RELEASE
  echo ******************
  goto prod_build
)

if %1 == test  (
  echo CREATE TEST RELEASE
  echo ******************
  goto test_build
)

if %1 == dev  (
  echo CREATE DEV RELEASE
  echo ******************
  goto dev_build
)

:invalid
echo ******************
echo Please enter valid input
echo Input parameters are: prod, test, dev or setvar
goto end
        

:prod_build

@echo on
echo #define DOWNLOAD_SOURCE "home" > ..\downloadsource.h
devenv ..\..\AdPlugin.sln /rebuild "Release Production"
"%pathAdvancedInstaller%\AdvancedInstaller.com" /edit adblock.aip /SetVersion %version%
"%pathAdvancedInstaller%\AdvancedInstaller.com" /rebuild adblock.aip
copy adblock.msi downloadfiles\simpleadblock.msi
copy adblock.msi installers\simpleadblock%version%.msi

"%pathAdvancedInstaller%\AdvancedInstaller.com" /edit adblockupdate.aip /SetVersion %version%
"%pathAdvancedInstaller%\AdvancedInstaller.com" /rebuild adblockupdate.aip
copy adblockupdate.msi downloadfiles\simpleadblockupdate.msi
copy adblockupdate.msi installers\simpleadblockupdate%version%.msi

echo %version%;%release%;%date%;%1;%comment% >> installers\simpleadblock_buildlog.dat
@echo off
goto end

:test_build

@echo on
echo #define DOWNLOAD_SOURCE "test" > ..\downloadsource.h
REM devenv ..\..\AdPlugin.sln /rebuild "Release Test"
"%pathAdvancedInstaller%\AdvancedInstaller.com" /edit adblock.aip /SetVersion %version%
"%pathAdvancedInstaller%\AdvancedInstaller.com" /rebuild adblock.aip
copy adblock.msi downloadfiles\simpleadblocktest.msi

"%pathAdvancedInstaller%\AdvancedInstaller.com" /edit adblockupdate.aip /SetVersion %version%
"%pathAdvancedInstaller%\AdvancedInstaller.com" /rebuild adblockupdate.aip
copy adblockupdate.msi downloadfiles\simpleadblocktestupdate.msi

echo %version%;%release%;%date%;%1;%comment% >> installers\simpleadblock_buildlog.dat
@echo off
goto end

:dev_build
@echo on
echo #define DOWNLOAD_SOURCE "dev" > ..\downloadsource.h
devenv ..\..\AdPlugin.sln /rebuild "Release Development"
"%pathAdvancedInstaller%\AdvancedInstaller.com" /edit adblock.aip /SetVersion %version%
"%pathAdvancedInstaller%\AdvancedInstaller.com" /rebuild adblock.aip
copy adblock.msi downloadfiles\simpleadblockdevelopment.msi

"%pathAdvancedInstaller%\AdvancedInstaller.com" /edit adblockupdate.aip /SetVersion %version%
"%pathAdvancedInstaller%\AdvancedInstaller.com" /rebuild adblockupdate.aip
copy adblockupdate.msi downloadfiles\simpleadblockdevelopmentupdate.msi

echo %version%;%release%;%date%;%1;%comment% >> installers\simpleadblock_buildlog.dat
@echo off
goto end

:end
@echo on
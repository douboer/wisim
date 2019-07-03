@SET VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio .NET
@SET VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio .NET\VC7
@SET FrameworkDir=C:\Program Files\Microsoft Visual Studio .NET\EnterpriseFrameworks
@SET FrameworkVersion=v1.1
@SET FrameworkSDKDir=C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK
@if "%VSINSTALLDIR%"=="" goto error_no_VSINSTALLDIR
@if "%VCINSTALLDIR%"=="" goto error_no_VCINSTALLDIR

@echo Setting environment for using Microsoft Visual Studio 2005 x86 tools.

@rem
@rem Root of Visual Studio IDE installed files.
@rem
@set DevEnvDir=C:\Program Files\Microsoft Visual Studio .NET\Common7\IDE

@set PATH=C:\Program Files\Microsoft Visual Studio .NET\Common7\IDE;C:\Program Files\Microsoft Visual Studio .NET\Vc7\bin;C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools;C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools\bin;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\bin;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\bin;C:\Program Files\Microsoft Visual Studio .NET\Vc7\vcpackages;%PATH%
@set INCLUDE=C:\Program Files\Microsoft Visual Studio .NET\Vc7\ATLMFC\INCLUDE;C:\Program Files\Microsoft Visual Studio .NET\Vc7\INCLUDE;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\include;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\include;%INCLUDE%
@set LIB=C:\Program Files\Microsoft Visual Studio .NET\Vc7\ATLMFC\LIB;C:\Program Files\Microsoft Visual Studio .NET\Vc7\LIB;C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\lib;C:\Program Files\Microsoft Visual Studio .NET\FrameworkSDK\lib;%LIB%
@set LIBPATH=C:\Program Files\Microsoft Visual Studio .NET\EnterpriseFrameworks;C:\Program Files\Microsoft Visual Studio .NET\Vc7\ATLMFC\LIB

@goto end

:error_no_VSINSTALLDIR
@echo ERROR: VSINSTALLDIR variable is not set. 
@goto end

:error_no_VCINSTALLDIR
@echo ERROR: VCINSTALLDIR variable is not set. 
@goto end

:end

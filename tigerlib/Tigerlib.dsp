# Microsoft Developer Studio Project File - Name="Tigerlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Tigerlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Tigerlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Tigerlib.mak" CFG="Tigerlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Tigerlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Tigerlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Tigerlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Tigerlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Tigerlib - Win32 Release"
# Name "Tigerlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\SCAN.CPP
# End Source File
# Begin Source File

SOURCE=.\tigerlib.cpp
# End Source File
# Begin Source File

SOURCE=.\TIGERRT1.CPP
# End Source File
# Begin Source File

SOURCE=.\TIGERRT2.CPP
# End Source File
# Begin Source File

SOURCE=.\TIGERRT4.CPP
# End Source File
# Begin Source File

SOURCE=.\TIGERRT5.CPP
# End Source File
# Begin Source File

SOURCE=.\TIGERRT6.CPP
# End Source File
# Begin Source File

SOURCE=.\tigerrt7.cpp
# End Source File
# Begin Source File

SOURCE=.\tigerrt8.cpp
# End Source File
# Begin Source File

SOURCE=.\tigerrtH.cpp
# End Source File
# Begin Source File

SOURCE=.\tigerrti.cpp
# End Source File
# Begin Source File

SOURCE=.\tigerRtZ.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\tigerlib.h
# End Source File
# Begin Source File

SOURCE=.\tigerrt1.h
# End Source File
# Begin Source File

SOURCE=.\tigerrt2.h
# End Source File
# Begin Source File

SOURCE=.\tigerrt4.h
# End Source File
# Begin Source File

SOURCE=.\tigerrt5.h
# End Source File
# Begin Source File

SOURCE=.\tigerrt6.h
# End Source File
# Begin Source File

SOURCE=.\tigerrt7.h
# End Source File
# Begin Source File

SOURCE=.\tigerrt8.h
# End Source File
# Begin Source File

SOURCE=.\tigerrtH.h
# End Source File
# Begin Source File

SOURCE=.\tigerrtI.h
# End Source File
# Begin Source File

SOURCE=.\tigerRtZ.h
# End Source File
# End Group
# End Target
# End Project

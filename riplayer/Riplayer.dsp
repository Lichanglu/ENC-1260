# Microsoft Developer Studio Project File - Name="Riplayer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Riplayer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Riplayer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Riplayer.mak" CFG="Riplayer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Riplayer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Riplayer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Riplayer - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /incremental:yes /debug /machine:I386

!ELSEIF  "$(CFG)" == "Riplayer - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Riplayer - Win32 Release"
# Name "Riplayer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CXPsytle"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Tools\ButtonXP.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools\ComboBoxXP.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools\Draw.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools\MenuXP.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools\ToolBarXP.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools\Tools.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools\XPEdit.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AboutDlg1.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\app_sdk_com.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\encoder_info.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_common.cpp
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_contain.cpp
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_interface.cpp
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_release.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\mpegts.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgBoxDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\parse_url.cpp
# End Source File
# Begin Source File

SOURCE=.\Picture.cpp
# End Source File
# Begin Source File

SOURCE=.\PictureDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayVedioDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\read_frame.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\record.cpp
# End Source File
# Begin Source File

SOURCE=.\RecordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Riplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Riplayer.rc
# End Source File
# Begin Source File

SOURCE=.\RiplayerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\rtp_stream.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\rtsp.cpp
# End Source File
# Begin Source File

SOURCE=.\SetDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StreamDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TipDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream\TS_stream.cpp
# End Source File
# Begin Source File

SOURCE=.\UserDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VedioDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "CXPsytle No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Tools\ButtonXP.h
# End Source File
# Begin Source File

SOURCE=.\Tools\ComboBoxXP.h
# End Source File
# Begin Source File

SOURCE=.\Tools\Draw.h
# End Source File
# Begin Source File

SOURCE=.\Tools\MenuXP.h
# End Source File
# Begin Source File

SOURCE=.\Tools\StatusBarXP.cpp
# End Source File
# Begin Source File

SOURCE=.\Tools\StatusBarXP.h
# End Source File
# Begin Source File

SOURCE=.\Tools\ToolbarXP.h
# End Source File
# Begin Source File

SOURCE=.\Tools\Tools.h
# End Source File
# Begin Source File

SOURCE=.\Tools\XPEdit.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AboutDlg1.h
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\Stream\middle_control.h
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_common.h
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_contain.h
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_interface.h
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_lib.h
# End Source File
# Begin Source File

SOURCE=.\mp4\mp4_release.h
# End Source File
# Begin Source File

SOURCE=.\Stream\mpegts.h
# End Source File
# Begin Source File

SOURCE=.\MsgBoxDlg.h
# End Source File
# Begin Source File

SOURCE=.\Picture.h
# End Source File
# Begin Source File

SOURCE=.\PictureDlg.h
# End Source File
# Begin Source File

SOURCE=.\PlayVedioDlg.h
# End Source File
# Begin Source File

SOURCE=.\Stream\read_freame.h
# End Source File
# Begin Source File

SOURCE=.\Stream\record.h
# End Source File
# Begin Source File

SOURCE=.\RecordDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Riplayer.h
# End Source File
# Begin Source File

SOURCE=.\RiplayerDlg.h
# End Source File
# Begin Source File

SOURCE=.\RMedia.h
# End Source File
# Begin Source File

SOURCE=.\SaveAsJpg.h
# End Source File
# Begin Source File

SOURCE=.\SetDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Stream\Stream.h
# End Source File
# Begin Source File

SOURCE=.\StreamDlg.h
# End Source File
# Begin Source File

SOURCE=.\TipDlg.h
# End Source File
# Begin Source File

SOURCE=.\UserDlg.h
# End Source File
# Begin Source File

SOURCE=.\VedioDlg.h
# End Source File
# Begin Source File

SOURCE=.\VoiceDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\01_1_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\01_1_2.jpg
# End Source File
# Begin Source File

SOURCE=.\res\01_1_2_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\01_1_3.jpg
# End Source File
# Begin Source File

SOURCE=.\res\01_1_4.jpg
# End Source File
# Begin Source File

SOURCE=.\res\01_1_5.jpg
# End Source File
# Begin Source File

SOURCE=.\res\01_1_6.jpg
# End Source File
# Begin Source File

SOURCE=.\res\02.jpg
# End Source File
# Begin Source File

SOURCE=.\res\03.jpg
# End Source File
# Begin Source File

SOURCE=.\res\04.jpg
# End Source File
# Begin Source File

SOURCE=.\res\05.jpg
# End Source File
# Begin Source File

SOURCE=.\res\06.jpg
# End Source File
# Begin Source File

SOURCE=.\res\07.jpg
# End Source File
# Begin Source File

SOURCE=.\res\08.jpg
# End Source File
# Begin Source File

SOURCE=.\res\09.jpg
# End Source File
# Begin Source File

SOURCE=.\res\10.jpg
# End Source File
# Begin Source File

SOURCE=.\res\10_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\11.jpg
# End Source File
# Begin Source File

SOURCE=.\res\12.jpg
# End Source File
# Begin Source File

SOURCE=.\res\13.jpg
# End Source File
# Begin Source File

SOURCE=.\res\14.jpg
# End Source File
# Begin Source File

SOURCE=.\res\14_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\15.jpg
# End Source File
# Begin Source File

SOURCE=.\res\15_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\16.jpg
# End Source File
# Begin Source File

SOURCE=.\res\16_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\17.jpg
# End Source File
# Begin Source File

SOURCE=.\res\17_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\18.jpg
# End Source File
# Begin Source File

SOURCE=.\res\19.jpg
# End Source File
# Begin Source File

SOURCE=.\res\1_1_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\20.jpg
# End Source File
# Begin Source File

SOURCE=.\res\21.jpg
# End Source File
# Begin Source File

SOURCE=.\res\22.jpg
# End Source File
# Begin Source File

SOURCE=.\res\22_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_10.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_12.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_13.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_14.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_15.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_16.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_17.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_17_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_18.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_2.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_3.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_4.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_5.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_6.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_7.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_1_8.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_10.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_11.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_12.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_13.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_14.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_15.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_16.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_17.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_18.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_2.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_3.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_4.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_5.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_7.jpg
# End Source File
# Begin Source File

SOURCE=.\res\2_2_8.jpg
# End Source File
# Begin Source File

SOURCE=.\res\32.ico
# End Source File
# Begin Source File

SOURCE=.\res\3232.ICO
# End Source File
# Begin Source File

SOURCE=.\res\6_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\7_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\8_1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\9_1.jpg
# End Source File
# Begin Source File

SOURCE=".\res\about(06-18-10-33-07).jpg"
# End Source File
# Begin Source File

SOURCE=".\res\about(06-18-10-33-07)_1.jpg"
# End Source File
# Begin Source File

SOURCE=".\res\about(06-18-10-33-07)_2.jpg"
# End Source File
# Begin Source File

SOURCE=.\res\CDRom.ico
# End Source File
# Begin Source File

SOURCE=.\res\centerright.jpg
# End Source File
# Begin Source File

SOURCE=.\res\centerright2.jpg
# End Source File
# Begin Source File

SOURCE=.\res\ico.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico256.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico3232.ico
# End Source File
# Begin Source File

SOURCE=".\res\Image Capture.ico"
# End Source File
# Begin Source File

SOURCE=.\res\jpg1.bin
# End Source File
# Begin Source File

SOURCE=.\res\jpg3.bin
# End Source File
# Begin Source File

SOURCE=.\res\jpg8.bin
# End Source File
# Begin Source File

SOURCE=.\res\jpg9.bin
# End Source File
# Begin Source File

SOURCE=".\res\left center.jpg"
# End Source File
# Begin Source File

SOURCE=.\res\left.jpg
# End Source File
# Begin Source File

SOURCE=.\res\LogOff.ico
# End Source File
# Begin Source File

SOURCE=.\res\Open.ico
# End Source File
# Begin Source File

SOURCE=.\res\right.jpg
# End Source File
# Begin Source File

SOURCE=.\res\Riplayer.ico
# End Source File
# Begin Source File

SOURCE=.\res\Riplayer.rc2
# End Source File
# Begin Source File

SOURCE=.\res\Riplayer_01.ico
# End Source File
# Begin Source File

SOURCE=.\res\Sound.ico
# End Source File
# Begin Source File

SOURCE=".\res\start-here.ico"
# End Source File
# Begin Source File

SOURCE=.\res\tool.ico
# End Source File
# Begin Source File

SOURCE=".\res\²¥·ÅÆ÷.jpg"
# End Source File
# Begin Source File

SOURCE=".\res\ÏîÄ¿2.ico"
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\res\xpstyle.manifest.txt
# End Source File
# End Target
# End Project

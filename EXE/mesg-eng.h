﻿/*=============================================================================
/                                   メッセージ
/
/============================================================================
/ Copyright (C) 1997-2023 Sota. All rights reserved.
/
/ Redistribution and use in source and binary forms, with or without
/ modification, are permitted provided that the following conditions
/ are met:
/
/  1. Redistributions of source code must retain the above copyright
/     notice, this list of conditions and the following disclaimer.
/  2. Redistributions in binary form must reproduce the above copyright
/     notice, this list of conditions and the following disclaimer in the
/     documentation and/or other materials provided with the distribution.
/
/ THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
/ IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
/ OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
/ IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
/ INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
/ USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
/ ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
/ THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/============================================================================*/

#define MSGJPN_0    _T("Backup start at : %s")
#define MSGJPN_1    _T("Backup from     : %s")
#define MSGJPN_2    _T("Backup to       : %s")
#define MSGJPN_3    _T("Backup done at  : %s")
#define MSGJPN_4    _T("Sun")
#define MSGJPN_5    _T("Mon")
#define MSGJPN_6    _T("Tue")
#define MSGJPN_7    _T("Wed")
#define MSGJPN_8    _T("Thu")
#define MSGJPN_9    _T("Fri")
#define MSGJPN_10   _T("Sat")
#define MSGJPN_11   _T("%d/%d/%d (%s) %d:%d")
#define MSGJPN_12   _T("Backup destination not specified.")
#define MSGJPN_13   _T("Restart Backup to restore settings complete.")
#define MSGJPN_14   _T("Pattern name not specified.")
#define MSGJPN_15   _T("Invalid option. (%s)")
#define MSGJPN_16   _T("Too many pathes. (%s)")
#define MSGJPN_17   _T("Cannot specify Path name with pattern name. (%s)")
#define MSGJPN_18   _T("Show Window")
#define MSGJPN_19   _T("Exit")
#define MSGJPN_20   _T("Backup pattern not found. (%s)")
#define MSGJPN_21   _T("Log")
#define MSGJPN_22   _T("Timer")
#define MSGJPN_23   _T("Option")
#define MSGJPN_24   _T("Option")
#define MSGJPN_25   _T("Log File")
#define MSGJPN_26   _T("All file(*.*)\0*\0")
#define MSGJPN_27   _T("Name")
#define MSGJPN_28   _T("Backup From")
#define MSGJPN_29   _T("Ignore 1")
#define MSGJPN_30   _T("Option")
#define MSGJPN_31   _T("Pattern Setting")
#define MSGJPN_32   _T("File Size")
#define MSGJPN_33   _T("Backup to")
#define MSGJPN_34   _T("Select File")
#define MSGJPN_36   _T("Backup From")
#define MSGJPN_37   _T("Only folder name accepted.\n\n%d files are ignored.")
#define MSGJPN_38   _T("Only filename accepted.\n\n%d folders are ignored.")
#define MSGJPN_39   _T("Import settings of the Backup Ver0.22 or earlier.\n")
#define MSGJPN_40   _T("The way of a backup pattern settings has changed.\n")
#define MSGJPN_41   _T("Please check pattern settings.")
#define MSGJPN_42   _T("Save Settings")
#define MSGJPN_43   _T("Reg file\0*.reg\0All file(*.*)\0*\0")
#define MSGJPN_44   _T("Failed to execute Registory Editor.")
#define MSGJPN_45   _T("Backup-settings.ini")
#define MSGJPN_47   _T("INI file\0*.ini\0All file(*.*)\0*\0")
#define MSGJPN_48   _T("Load Settings")
#define MSGJPN_49   _T("Reg/Ini file\0*.reg;*.ini\0All file(*.*)\0*\0")
#define MSGJPN_51   _T("File must be *.reg or *.ini.")
#define MSGJPN_52   "# Do not edit this file.\n"    /* ANSI */
#define MSGJPN_53   _T("Can't save to INI file.")
#define MSGJPN_54   _T("&Cancel")
#define MSGJPN_55   _T("&Repeat Backup")
#define MSGJPN_56   _T("R&estart")
#define MSGJPN_57   _T("&Repeat Backup (%d minutes)")
#define MSGJPN_58   _T("In Progress")
#define MSGJPN_60   _T("Please Wait")
#define MSGJPN_61   _T("ERROR: Volume lavel not match.")
#define MSGJPN_62   _T("ERROR: Backup destination not found. (%s)")
#define MSGJPN_63   _T("--- Searching Folders ---")
#define MSGJPN_64   _T("--- Deleting Disused Folders ---")
#define MSGJPN_65   _T("--- Deleting Disused Files ---")
#define MSGJPN_66   _T("--- Creating Folders ---")
#define MSGJPN_67   _T("--- Copying Files ---")
#define MSGJPN_68   _T("Done. (Error:%d, Total Errors:%d) (Create:%d, Delete:%d, Copy:%d)")
#define MSGJPN_69   _T("Abort. (Error:%d, Total Errors:%d) (Create:%d, Delete:%d, Copy:%d)")
#define MSGJPN_70   _T("ERROR: Cannot create %s. (%s)")
#define MSGJPN_71   _T("Make : %s")
#define MSGJPN_73   _T("  Change caps. %s\n")
#define MSGJPN_74   _T("Name : %s")
#define MSGJPN_75   _T("  Chattr. %s\n")
#define MSGJPN_76   _T("Attr : %s (Src=0x%x : Dst=0x%x)")
#define MSGJPN_77   _T("Updt : %s")
#define MSGJPN_78   _T("Copy : %s")
#define MSGJPN_79   _T("ERROR: Disk is full.")
#define MSGJPN_80   _T("ERROR: Cannot copy %s. (%s)")
#define MSGJPN_81   _T("Dele : %s")
#define MSGJPN_82   _T("ERROR: Cannot delete %s. (%s)")
#define MSGJPN_83   _T("ERROR: %s not found.")
#define MSGJPN_84   _T("Please specify backup source and destination.")
#define MSGJPN_85   _T("Delete selected pattern. Sure?")
#define MSGJPN_86   _T("Please specify backup source.")
#define MSGJPN_87   _T("Cannot execute log file viewer.")
#define MSGJPN_88   _T("Execute File")
#define MSGJPN_89   _T("Exe file\0*.exe\0All file(*.*)\0*\0")
#define MSGJPN_90   _T("Cannot change Windows power state.")
#define MSGJPN_91   _T("System")
#define MSGJPN_92   _T("%d seconds to go.")
#define MSGJPN_93   _T("Do nothing")
#define MSGJPN_94   _T("Quit the program")
#define MSGJPN_95   _T("Shut down the computer")
#define MSGJPN_96   _T("Goes into standby mode")
#define MSGJPN_97   _T("Hibernate the computer")
#define MSGJPN_98   _T("Shutting down the computer.")
#define MSGJPN_99   _T("Computer goes into standby.")
#define MSGJPN_100  _T("Hibernating the computer.")
#define MSGJPN_101  _T("Quit and goes into standby")
#define MSGJPN_102  _T("Quit and hibernate")
#define MSGJPN_103  _T("Backup To")
#define MSGJPN_104  _T("Sound File")
#define MSGJPN_105  _T("Wave file\0*.wav\0All file(*.*)\0*\0")
#define MSGJPN_106  _T("Cannot create folder because file already exist and cannot delete this file.")
#define MSGJPN_107  _T("Searching.")
#define MSGJPN_108  _T("Finished.")
#define MSGJPN_109  _T("Advanced")
#define MSGJPN_110  _T("Replace Pathnames")
#define MSGJPN_111  _T("Failed to get volume label")
#define MSGJPN_112  _T("No check")
#define MSGJPN_113  _T("Sort")
#define MSGJPN_114  _T("Overwrite following file%s?")
#define MSGJPN_115  _T(" with newer file")
#define MSGJPN_116  _T(" with older file")
#define MSGJPN_117  _T(" with different file")
#define MSGJPN_118  _T("The error can be confirmed by Log menu.")
#define MSGJPN_119  _T("Log is not existing.")
#define MSGJPN_120  _T("Goes into sleep mode")
#define MSGJPN_121  _T("Quit and Goes into standby mode")
#define MSGJPN_122  _T("Computer goes into sleep.")
#define MSGJPN_123  _T("&Pause")
#define MSGJPN_124  _T("Ignore 2")
#define MSGJPN_125  _T("--- Preparing backup ---")
#define MSGJPN_126  _T("Updt : %s (Size is not same : Src=%s, Dst=%s)")
#define MSGJPN_127  _T("Updt : %s (Time stamp is not same : Src=%s, Dst=%s)")
#define MSGJPN_128  _T("<< No pattern name >>")
#define MSGJPN_129  _T("ERROR: Some error occurd while searching %s.(%s)")
#define MSGJPN_130  _T("Deletion file move to")
#define MSGJPN_131  _T("   --> Move file to %s")
#define MSGJPN_132  _T("ERROR: File move error (%s -> %s)")
#define MSGJPN_133  _T("Pattern Name：%s")
#define MSGJPN_134  _T("%d:%d:%d")
#define MSGJPN_135  _T("Process Time：%s")
#define MSGJPN_136  _T("Battery threshold not specified.")
#if USE_SAME_AS_SUCCESS
#define MSGJPN_137  _T("Same as on success")
#endif /* USE_SAME_AS_SUCCESS */
#define MSGJPN_138  _T("Skip (%s)")
#define MSGJPN_139  _T("Skip. (Error:%d, Total Errors:%d) (Create:%d, Delete:%d, Copy:%d)")
#define MSGJPN_140  _T("-- Disabled -- ")
#define MSGJPN_141  _T("Folder not Copied")
#define MSGJPN_142  _T("MTP devices not connected on the system.")
#define MSGJPN_143  _T("Choose MTP device's folder for destination.")
#define MSGJPN_144  _T("Double click item to expand child items.")
#define MSGJPN_145  _T("ERROR: MTP device (%s) not found.")
#define MSGJPN_146  _T("ERROR: Folder (%s) not found on the MTP device.")
#define MSGJPN_147  _T("ERROR: Specified dackup destination is invalid.")
#define MSGJPN_148  _T("Getting file structure from destination device.")
#define MSGJPN_149  _T("WARNING: Cannot change folder name. Folder will remain under its previous name. (%s -> %s)")
#define MSGJPN_150  _T("WARNING: Cannot change file name. Copy this file to destination.")
#define MSGJPN_151  _T("ERROR: Cannot open MTP device (%s)")

#!/bin/bash

BACKUP_ROOT=$(realpath ".")
BACKUP_FILES=(universe/ serverconfig.txt)

mkdir cmdchat_backup
cd cmdchat_backup

# Keep last 100 backups
ls -1t backup_si_server_* | tail -n +100 | xargs rm

timestamp=$(date "+%Y_%m_%d_%H_%M_%S")
TAR_FILENAME="backup_sids_$timestamp.tar"

tar -C $BACKUP_ROOT/ -cf $TAR_FILENAME $BACKUP_FILES

#!/usr/bin/awk -f
# SPDX-License-Identifier: GPL-2.0
# extract linker version number from stdin and turn into single number
	{
	gsub(".*\\)", "");
<<<<<<< HEAD
=======
	gsub(".*version ", "");
	gsub("-.*", "");
>>>>>>> temp
	split($1,a, ".");
	print a[1]*100000000 + a[2]*1000000 + a[3]*10000;
	exit
	}

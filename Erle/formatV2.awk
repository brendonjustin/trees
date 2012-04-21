#!/bin/awk -f

# Modifies the Erle file, should result in something with materials
# Will require some modification afterwards
{
    FS="[/ ]";
    if ($1 == "f")
    {
	print "f",$2,$5,$8;
    }
    else if ($1 == "v" || $1 == "vt" || $1 == "usemtl")
    {
	print $0;
    }
}

#!/bin/awk -f
{
    FS="[/ ]";
    if ($1 == "f")
    {
	print "f",$2,$5,$8;
    }
    else if ($1 == "v")
    {
	print $0;
    }
}

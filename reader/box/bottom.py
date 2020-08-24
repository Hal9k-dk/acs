#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import re

# Assumes SolidPython is in site-packages or elsewhwere in sys.path
from solid import *
from solid.utils import *

SEGMENTS = 16

case_th = 1.5
case_h = 42
case_d = 18
case_w = 160
coil_sup_l = 3
coil_sup_d = 3
# Inner diameter of coil
coil_sup_w = 43
coil_sup_h = 31
# Width of screw block
sw = 12
# Thickness of plate
front_th = 2

def bottom():
    return color(Red)(translate([-(case_w + 2*sw)/2, -case_h/2, 0])(cube([case_w + 2*sw, case_h, front_th])))

def frame():
    ow = case_w - 2*case_th - .5
    oh = case_h - 2*case_th
    fr_h = 5
    outer = translate([-ow/2, -oh/2, front_th])(cube([ow, oh, fr_h]))
    iw = ow - 2*case_th
    ih = oh - 2*case_th
    inner = translate([-iw/2, -ih/2, front_th])(cube([iw, ih, fr_h+2]))
    ch = oh + 2
    cw = 112
    cutout = translate([-32, -ch/2, front_th + 2])(cube([cw, ch, fr_h]))
    return outer-inner - cutout

def screw_hole(left):
    offset = case_th/2
    if not left:
        offset = -offset
    return translate([-sw/2, -case_h/2, -1])(translate([sw/2+offset, case_h/2, 0])(cylinder(h=case_h+2, r=2)))

def assembly():
    bt = bottom()
    fr = frame()
    s1 = left(case_w/2+sw/2-0.1)(screw_hole(True))
    s2 = right(case_w/2+sw/2-0.1)(screw_hole(False))
    return fr + bt - s1 - s2

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)

# Local Variables:
# compile-command: "python bottom.py"
# End:

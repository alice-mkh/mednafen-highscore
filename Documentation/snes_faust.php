<?php require("docgen.inc"); ?>

<?php BeginPage('snes_faust', 'Super Nintendo Entertainment System/Super Famicom'); ?>

<?php BeginSection('Introduction', 'Section_intro'); ?>
<p>
The "<b>snes_faust</b>" emulation module is experimental, and not used automatically by default except for SPC and SNSF playback.  To use this module rather than the "<b><a href="snes.html">snes</a></b>" module, you must either set the "snes.enable" setting to "0", or pass "-force_module snes_faust" to Mednafen each time it is invoked.
</p>
<p>
Timing is approximate, so some games may exhibit timing-related issues.  The only input devices currently emulated are the standard SNES gamepad, mouse, and multitap.  The following special cart chips and devices are emulated:
<ul>
 <li>CX4
 <li>DSP-1 (HLE, currently unsuitable for use with netplay)
 <li>DSP-2 (HLE)
 <li>MSU1
 <li>S-DD1
 <li>SA1
 <li>Super FX
</ul>
</p>
<p>
A unique feature to this module(at the current time) is optional 1-frame speculative execution, disabled by default, controlled by the <a href="#snes_faust.spex">snes_faust.spex</a>
setting.  Enabling it will reduce input(controller)->output(video) latency by 1 video frame(~16.7ms), with no deleterious effects on most games tested(though it will increase CPU usage a bit).  Combine it
with the setting changes <a href="mednafen.html#Section_minimize_video_lag">recommended here</a> for a better netplay experience.
</p>

<?php EndSection(); ?>

<?php PrintSettings(); ?>

<?php EndPage(); ?>


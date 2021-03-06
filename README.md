Accurate Radio Transmission Using Photon Mapping
================================================

Getting the Code
----------------
https://github.com/laprej/photons will show available options for viewing
and downloading the code as you see fit.  Clicking on "Downloads" will
present you with several options including "Download .tar.gz" and
"Download .zip."  Additionally, you can use git directly:

    git clone git://github.com/laprej/photons.git

Running the Code
----------------
To build the code, simply type:

    make

This should build the code.  Assuming no errors, it can be run like so:

    ./render <parameters>

Typically parameters involve input files and number of photons to shoot. A
good start would be something like this:

    ./render -input refloormapsobj/AE_quads.obj -num_photons_to_shoot 500

Additionally, we added the **d** command.  Once the photons have completed
firing, i.e. after the **p** key has been pushed and it has completed, pushing
the **d** key will light the spheres according to a conventional color scheme.
Green is good and means the sphere received enough photons for communication
to take place.  Red is bad and means it did not receive enough photons.  Colors
will vary between these two extremes as appropriate.

To repeat our experiments, you may run the following commands:

    ./render -input refloormapsobj/AE_Quads_Control.obj -num_photons_to_shoot 10000
	./render -input refloormapsobj/AE_Quads_No_Transmit.obj -num_photons_to_shoot 10000

Of course, shooting 10000 photons will take quite a while.  Consider
using a smaller number (such as 500 or 1000).

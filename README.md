# Dynamic Convolution Plugin
The Dynamic Convolution plugin is a convolution plugin with additional controls over the location of the file being convolved. It allows you to upload a selcted file of any standard format (.aiff, .wav) and performs real-time convolution on the input with the file. The location of the file where convolution is being performed can be changed in-real time (although changes are not smooth). It was built using the JUCE framework. 

The project source can be found on my Github here:
https://github.com/Bohr33/Dynamic_Convolution_Plugin.git

### Building the Plugin
In the repository you will find the source code and the .jucer file. To build the plugin you will need JUCE, and the proJucer installed. Once these are installed, you can launch the project via the Pro-Jucer and open in your selected IDE. From there, simply build the project and it should appear as an available VST or AU plugin, depending on your selected build type.

## Controls
There are 3 main controls to the plugin.

**File Position**: This controls where convolution of the file will begin. Changes to this control will be shown and updated on the file display window.

**File Length**: This controls how far into the file you would like to convolve with. Updates to this will also be displayed on the file display window.

**Dry/Wet**: This controls the balance between the input and the convolution output.

To upload a file, simply press the "Open" button below the file display window and select a file. 

## Implementation
The convolution process is implemented via a partitioned convolution algorithm. Details on this process can be found in the Dynamic_Convolution.cpp and header file. 

### Dynamic_Convolution Class
The Dynamic_Convolution class performs the bulk of the processing. A custom structure was created to manage the buffers used for the convolution process. This structure manages one channel of the convolution process, so two are used for stereo processing. Each processing function takes the Convolution channel structure as an argument.

A general outline of the process is as follows:

1. On a new file load, read the file and split into buffer sized chunks. Store the chunks into a matrix and resize the matrix. Then perform an FFT on the chunks to obtain the signal in the frequency domain.
2. For each new buffer block, perform an FFT on the input, then store the input into a buffer.
3. Next, read through the entire input buffer, and multiply each partition of the IR with the respective input buffer chunk and store in an output buffer.
4. Take the inverse FFT of the sum to obtain a time domain signal of size 2 * buffer size.
5. With the output signal, perform the overlap add process by adding the first half with the last of the previous buffer, and store the last half in the overlap buffer.
6. Copy the result into the Juce Audio buffer, where it will be sent to the output.


### Limitations
The Dynamic Convolver does support both mono and stereo files for convolution, however it is limited in its processing capabilities. There is currently no formal check if the file length is too long, so if too large of a file is uploaded, the processing will simply cutout. However, the controls can shorten the selection in real-time which will allow processing to continue. This is especially apparent with stereo files as twice as much processing is needed for the same amount of time. 

### Notes

**Other Files**: There are currently some extra files in the source folder which are not being used: Convolution.cpp and Convolution.h. These were basic classes used in the beginning of this project to test the convolution process provided by JUCE, and a initial version of the now Dynamic_Convolution class.









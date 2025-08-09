# Autotests

Automated testing for plugins to allow 
[`QImage`](https://doc.qt.io/qt-6/qimage.html) to support
extra file formats.

## Introduction

The testing part is essential for the correct functioning of the plugins. 
There are generic read/write tests and specific tests for the plugins that
require them.

## Read tests

The generic reading tests are contained in the `read` folder. Inside the 
`read` folder, there are one or more folders for each plugin to be tested.
A plugin can support multiple image file types, and if you need different 
parameters for each type, you need to create a folder for each type supported
by the plugin (see e.g. HEIF plugin). If all formats supported by the plugin 
do not require different parameters, a single folder approach is simpler (see 
e.g. PSD plugin).

The reading tests are mainly based on comparing the image read by the plugin 
with a template in a known and working format. For this reason, the template 
formats chosen are those distributed by the Qt project: PNG in the first 
instance and TIFF for image formats not supported by PNG (e.g. CMYK images).

Some image options such as `QImageIOHandler::Size`, 
`QImageIOHandler::ImageFormat` and `QImageIOHandler::ImageTransformation` are 
also checked. If supported by the plugin, the resulting image is checked to 
see if it is compatible with the option's specification.

Optionally, for each image, you can also create a JSON file to modify the test 
behavior and/or verify data other than image pixels, such as metadata.

Finally, two tests are run for each test case:
- On a random access device: this test must not fail.
- On a sequential access device: a plugin may not support sequential operation. 
In this case, the test is skipped. However, if an image is returned, the test 
must succeed.

### The readtest command

To start a test, run the `readtest` command with the format to test as an 
argument. The format is one of those supported by plugins and a folder with 
the name of the format must be present inside the `read` folder.
Depending on the format, you can specify the following additional options.

- `--help`: Displays help on commandline options.
- `--fuzz <max>`: The fuzziness. Used to add some deviation in ARGB data 
(nornally used on lossy codec).
- `--perceptive-fuzz`: Used to scale dynamically the fuzziness based on 
the alpha channel value. This is useful on images with pre-multiplied and 
small alphas. Qt can use different roundings based on optimizations resulting 
in very different RGB values. Since the alpha is small visually there is no 
difference (so it is not considered an error).
- `--skip-optional-tests`: Used to skip the optional test such as metadata 
and resolution tests.

Note that some tests may fail if the correct options are not used. The correct
options for each test are defined in [CMakeLists.txt](CMakeLists.txt).
See also [Add a test to CMakeLists.txt](#add-a-test-to-cmakeliststxt).

### Test image nomenclature

Each test consists of the image to test, the verification image(s) (template)
and, optionally, the additional JSON file.
To be a test, the names of these files must be the same (except for 
the extension). A test for a JXL image would be, for example, composed like 
this:
- `testRGB.jxl`: The image to test.
- `testRGB.jxl.json`: The test behavior modifier (note that it must contain 
the double extension).
- `testRGB.png`: How the image should look (template). The template name 
can be different if specified in the JSON file.

Although there is no precise rule for the name of a test, it is good to have 
a name that is explanatory.

### JSON behavior file

The behavior file was initially introduced to solve compatibility issues 
between different versions of Qt supported by the framework. It was later 
extended to also check image metadata.

The JSON file consists of an array of JSON objects. The objects in the array 
are iterated sequentially and the first object that matches the requirements 
is used for testing (successes are ignored).

Supported values ​​for JSON objects:
- `comment`: Type string. A string shown by the test when a condition occurs.
- `description`: Type string. A description of the object. Not used by the 
test.
- `disableAutoTransform`: Type boolean. By default, tests are run with 
autotransform enabled (i.e. rotation is applied if the plugin supports it).
Set to `true` to disable autotransform.
- `filename`: Type string. Name of the template file to use. E.g. 
"testRGB_Qt_6_2.png".
- `fuzziness`: Type integer. Set the fuzziness only if not already set on the 
command line. The value set on the command line wins over the one in the JSON 
file.
- `maxQtVersion`: Type string. Maximum Qt version this object is compatible 
with (if not set means all). E.g. "6.2.99".
- `metadata`: Type Array. An array of key/value objects (string type) 
containing the image metadata as returned by `QImage::text`.
- `minQtVersion`: Type string. Minimum Qt version this object is compatible 
with (if not set means all). E.g. "6.2.0".
- `perceptiveFuzziness` Type boolean. Set the perceptive fuzziness only if not
already set on the command line. The value set on the command line wins over 
the one in the JSON file.
- `resolution`: Type object. An object with the `dotsPerMeterX` and
`dotsPerMeterY` (integer) values ​​of the image.
- `seeAlso`: Type string. More info about the object. Normally used to point
to bug reports. Not used by the test.
- `unsupportedFormat`: Type `boolean`. When true, the test is skipped.

Some examples:
- Example 1: [Runs only on Qt without alpha bug on float formats](read/jxl/testcard_rgba_fp16.jxl.json)
- Example 2: [Rotation disabled](read/jxl/orientation6_notranfs.jxl.json)
- Example 3: [Metadata](read/psd/metadata.psd.json)
- Example 4: [Check Qt version, resolution and metadata](read/psd/mch-16bits.psd.json)
- Example 5: [Fuzziness setting](read/xcf/birthday16.xcf.json)
 
These are just a few examples. More examples can be found in the test folders.

## Write tests

The generic writing tests are contained in the `write/basic` and 
`write/format` folders. Similar to the read tests, they verify the written 
(and then reread) image with a template.

The write test is composed of several phases:
- Basic test: Uses the `write/basic` folder and checks the most common images 
and, optionally, metadata and resolution via a JSON properties file.
- Format test: Uses the `write/format` folder and checks that all QImage image
formats are written correctly.
- Dimensional test: Uses the `write/format` folder and check images of 
different sizes (odd numbers, prime numbers, etc.) to verify internal 
alignments.
- Null device test: Verify that there are no crashes if the device is null.

### The writetest command

To start a test, run the `writetest` command with the format to test as an 
argument. The format is one of those supported by plugins, a folder with 
the name of the format must be present inside the `write/format` folder and 
may need a template image in `write/basic`.
Depending on the format, you can specify the following additional options.

- `--help`: Displays help on commandline options.
- `--create-format-templates`: Create template images for all formats 
supported by the QImage in `write/format`. Command to simplify the creation of
format test images when adding a new plugin or modifying an old one. This 
command is not intended to be used from the CMakeLists file as it must be used
manually and the generated images must be verified one by one.
- `--fuzz <max>`: The fuzziness. Used to add some deviation in ARGB data 
(nornally used on lossy codec).
- `--lossless`: Check that reading back the data gives the same image.
- `--no-data-check`: Don't check that write data is exactly the same.
- `--skip-optional-tests`: Skip optional data tests (metadata, resolution,
etc...).

Note that some tests may fail if the correct options are not used. The correct
options for each test are defined in [CMakeLists.txt](CMakeLists.txt).
See also [Add a test to CMakeLists.txt](#add-a-test-to-cmakeliststxt).

### JSON properties file

The properties file must be located in `write/basic` and must have the name 
of the file format (e.g. jxl.json). It is a JSON object composed of the 
following values:
- `format`: Type string. The format tested.
- `metadata`: Type Array. An array of key/value objects (string type) 
containing the image metadata as returned by `QImage::text`.
- `resolution`: Type object. An object with the `dotsPerMeterX` and `
dotsPerMeterY` (integer) values ​​of the image.

[This is an example](write/basic/jxl.json) of property file.


## Custom tests

If the generic read/write tests do not meet the requirements of a plugin, 
it is possible to write a custom test. 
In general it makes sense to write a dedicated test for a format if and 
only if you are testing unique features not present in other plugins.

### The PIC test

The PIC test is generated using Qt Test class. For more information 
see [Qt Test](https://doc.qt.io/qt-6/qttest-index.html).

### The ANI test

The ANI test is generated using Qt Test class. For more information 
see [Qt Test](https://doc.qt.io/qt-6/qttest-index.html).


## Add a test to CMakeLists.txt

To add a test to CMake use the `kimageformats_read_tests` and 
`kimageformats_write_tests` functions. For example, to add the read 
tests for the PSD you just write `kimageformats_read_tests(psd)`.

It is also possible to pass command line arguments to the test by 
appropriately composing the string passed to the test functions.
For boolean parameters you need to add a string starting with '-' 
after the image format. For example, to pass `--skip-optional-tests` 
to the PSD plugin write `kimageformats_read_tests(psd-skipoptional)`.

To add a fuzziness of 4, you must first set it as follows: 
`kimageformats_read_tests(FUZZ 4 psd-skipoptional)`.

The possible modifiers for `kimageformats_read_tests` are as follows:
- `-skipoptional`: Add the `--skip-optional-tests` command line parameter.

The possible modifiers for `kimageformats_write_tests` are as follows:
- `-lossless`: Add the `--lossless` command line parameter.
- `-nodatacheck`: Add the `--no-data-check` command line parameter.
- `-skipoptional`: Add the `--skip-optional-tests` command line parameter.

To set multiple parameters, you can enter multiple modifiers. For example:
```
kimageformats_write_tests(FUZZ 1
    hej2-nodatacheck-lossless
)
```

## OSS-Fuzz

Plugins are also tested with [OSS-Fuzz](https://google.github.io/oss-fuzz/)
project to identify possible security issues. When adding a new plugin it is 
also necessary to add it to the test in the [kimageformats 
project](https://github.com/google/oss-fuzz/tree/master/projects/kimageformats) 
on OSS-Fuzz.

## TODO

List of tests not implemented or only partially implemented.

### Color Profiles Test

Many plugins support color profiles via [`QColorSpace`](https://doc.qt.io/qt-6/qcolorspace.html). 
Checking for correct color management is increasingly necessary especially now
that monitors are HDR and other than RGB color spaces have been added to Qt.

Furthermore, lossy plugins often have different color management behaviors 
depending on how the image is saved.

### Animations Test

Few plugins support animations. There are currently no plans for 
implementation.

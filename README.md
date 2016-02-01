# Meshlab Snapshot

[MeshLab](http://meshlab.sourceforge.net/) has not been updated since 02 April 2014, version 1.3.3. The commit logs, as of 01 February 2016, indicate that version 1.4 is seeing active development but there has not been any official news.

This repository is a single snapshot in time around the release of version 1.3.2.  Originally I kept a mirrored version for my own purposes but it was too much of a chore to keep the scripts working cleanly and eventually I stopped working on it.

Flash forward to 2016 and unfortunately MeshLab is still a chore to manually compile from source.  Compounding this problem is the lack of tags in the MeshLab and VCGLIB trunks making it nearly impossible to know which revisions build which version.  MeshLab also appears to be moving towards QT 5.X.

## Missing plugins

These plugins were removed from `meshlab_full.pro` to get a working version of MeshLab.

* filter_csd
* filter_plymc
* decorate_base
* decorate_background
* decorate_shadow
* decorate_raster_proj

## Getting MeshLab (Old Way)

```
svn checkout svn://svn.code.sf.net/p/meshlab/code/trunk meshlab-code
cd meshlab-code
svn checkout svn://svn.code.sf.net/p/vcg/code/trunk vcg-code
```
## Compiling MeshLab

Note:
Some plugins of MeshLab invokes functions exported by external libraries. You have to compile these libraries before attempting to compile the whole MeshLab's code.

### To compile MeshLab and all MeshLab plugins:

First compile the needed external libraries
```
cd MESHLAB_DIRECTORY/src/external
qmake -recursive external.pro
make
```

Then compile MeshLab and its plugins

```
cd MESHLAB_DIRECTORY/src/
qmake -recursive meshlab_full.pro
make
```

Alternatively you can directly compile only a subset of the whole meshlab by using the meshlab_mini.pro. This minimal subset does not require any external library (give a look at the .pro itself for more info).

```
cd MESHLAB_DIRECTORY/src/
qmake -recursive meshlab_mini.pro
make
```

For more complete info, please, refer to the meshlab wiki:
 http://sourceforge.net/p/meshlab/code/HEAD/tree/trunk/meshlab/src/

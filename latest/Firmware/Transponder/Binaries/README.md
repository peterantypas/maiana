## Pre-built binaries

This directory contains binaries for boards 11.3 and newer *purchased through the OpenMarine store*. If you do not use OpenPlotter 
or you are unsure whether these apply to your board, please start a ticket or a discussion.

### FW 4.1.0

This firmware release corrects the tuning of the crystal oscillator for the Si4467 chips. This applies to boards made after November 2021 which contain the following
crystal:

![IMG_1673](https://user-images.githubusercontent.com/1565933/156231414-5624cf9b-d8ff-4dc6-9451-eac7009387b8.jpg)

The tuning changed by 5 KHz when the design had to migrate to this crystal due to supply problems. See issue [#103](https://github.com/peterantypas/maiana/issues/103)


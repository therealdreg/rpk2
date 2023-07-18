## W25Q512JV Winbond Flash Driver

This project implements a generic driver for W25Q512JV flash memory over SPI using ATmega32u4. Nevertheless, it is design with a generic approach, making it easier to adapt the source code to different flash models or MCUs.

### Details

There are some details to take into consideration:
* When sending a sector erase instruction, it will align the address to the physical sector. For instance, `SPIF_4B_erase_sector(0x4)` will erase the first sector instead of a logical one starting at `0x4`.
* Similarly, using `SPIF_INST_4B_WRITE` will write only on the same physical page, therefore writing at `0xF8` 16 bytes instead of writing at the second page it would write remaining data at the beginning of the first page. This behavior is abstracted by the driver, so the developer does not have to deal with it.

### Implementation
There are two features to explain in order to understand how the driver works.

#### Temporal sector

Last sector is used to write already existing data to it, mixing it with new one. Finally, erasing involved sector and writing from the temporal one. This feature was needed due to the following:
* Needed to make this driver compatible with [ATmega32u4](https://www.microchip.com/en-us/product/ATMEGA32U4) which only has 2.5 KB SRAM. That amount of SRAM is not enough to store a whole sector by far.
* The least amount of data that can be erased is a sector.
* Write operations can only write `0` so if you just want to partially overwrite a sector, you need to both keep previous data and erase the whole sector.

#### Compatible writes

In order to speed up writing instructions, temporal sector is only used when necessary. To achieve so we first need to understand when a sector erase is a must.

| Buffer | Flash | Compatible |
|--------|-------|------------|
|   0    |   0   |     ✅    |
|   0    |   1   |     ✅    |
|   1    |   0   |     ❌    |
|   1    |   1   |     ✅    |

So in order to detect whether is we can safely write, we need a logic gate or a combination of them that gives a different result in such case.

There are multiple ways of solving that problem, here is the one used in the driver.

The truth table of the OR table shown below has a unique result when both values are 0, hence useful to identify a unique case.

| Buffer | Flash | Result |
|--------|-------|--------|
|   0    |   0   |   0    |
|   0    |   1   |   1    |
|   1    |   0   |   1    |
|   1    |   1   |   1    |

We want to identify when the buffer has a value of `1` and the flash a `0` but in order to identify the unique case both need a value of `0` so we simply apply a not operation on the buffer. Here is the resulting truth table:

| !Buffer | Flash | Result | Compatible |
|---------|-------|--------|------------|
|    1    |   0   |   1    |     ✅    |
|    1    |   1   |   1    |     ✅    |
|    0    |   0   |   0    |     ❌    |
|    0    |   1   |   1    |     ✅    |

### Mentions

Driver run and tested using [rpk2](https://rootkit.es/rpk2_doc/) by [Dreg](https://twitter.com/therealdreg).

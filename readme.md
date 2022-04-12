

## Introduction

Zip file format specification is intended to define a **cross-platform**, interoperable file format. Since its first publication in 1989, PKWARE has remained committed to ensuring the interoperability of the .ZIP file format through this specification. 

### General Format of a .ZIP file

Files stored in arbitrary order. 

Large `.ZIP` files can span multiple diskette media or be split into user-defined segment sizes. 

Overall `.ZIP` file format: 

- (FILE 1) Local file header
- (FILE 1) File data
- (FILE 1) Data descriptor [optional] 
- ... (FILE N) with the same orders. 
- Archive decryption header (EFS)
- Archive extra data record (EFS) 
- Central directory 
- Zip64 end of central directory record 
- Zip64 end of central directory locator 
- End of central directory record 

---

#### Local File Header

- local file header signature   4 bytes: (0x04034b50) 
- version needed to extract   2 bytes 
- general purpose bit flag   2 bytes
  - Bit 0: If set, indicates that the file is encrypted. 
  - Bit 1: 
    [If compression method used was type 6, Imploding] If set, indicates an 8K sliding dictionary was used. If clear, then a 4K sliding dictionary was used. 
  - Bit 2: [If compression method used was type 6, Imploding] If set, indicates 3 Shannon-Fano trees were used to encode the sliding dictionary output. If clear, then 2 Shannon-Fano trees were used. 
  - When Compression method is 8 and 9 - Deflating: 
    - Bit 2, Bit 1 (0 0): Normal (-en) compression option was used. 
    - (0, 1): Maximum (-exx/-ex) compression option was used. 
    - (1, 0): Fast (-ef) compression option was used. 
    - (1, 1): Super Fast (-es) compression option was used. 
  - When compression method is other, they are not defined. 
  - **Bit 3: If this bit is set, the fields crc-32, compressed size and uncompressed size are set to zero in the local header. The correct values are put in the data descriptor immediately following the compressed data.** 
  - Bit 4: Reserved for use with method 8, for enhanced deflating. 
  - Bit 5: If this bit is set, indicates that the file is compressed patched data. 
  - Bit 6: Strong encryption. If this bit is set, you should set the version needed to extract value to at least 50 and you must also set bit 0. 
    <u>If AES encryption is used, the version needed to extract value must be at least 51.</u> 
  - Bit 13: Used when encrypting the Central Directory to indicate selected data values in the local header are masked to hide their actual values. 
    <u>See the section describing the Strong Encryption Specification for details.</u> 
- compression method   2 bytes 
  - 0 - The file is stored (no compression). 
  - 1 - The file is Shrunk 
  - (2 - 5) - The file is reduced with compression factor (1 - 4)
  - 6 - The file is Imploded 
  - 7 - Reserved for Tokenizing compressing algorithm 
  - 8 - The file is Deflated 
  - <u>9 - Enhanced Deflating using Deflate64 (tm)</u> 
  - 10 - PKWARE Data Compression Library Imploding 
  - 11 - Reserved 
  - 12 - File is compressed using BZIP2 algorithm
- Data and Time Fields: 2 bytes each 
  - The data and time are encoded in standard MS-DOS format. 
  - If input came from standard input, the data and time are those at which compression was started for this data. 
  - If encrypting the central directory and general purpose bit flag 13 is set indicating masking, the value stored in the local header will be zero. 
- CRC-32: 4 bytes 
  - The magic number for the CRC is 0xdebb20e3. The proper CRC pre and post conditioning is used, meaning that the CRC register is pre-conditioned with all ones (a starting value of 0xffffffff) and the value is post-conditioned by taking the one's complement of the CRC residual. 
  - If encrypting the central directory and general purpose bit flag 13 is set indicating masking, the value stored in the Local Header will be zero. 
- compressed size: 4 bytes 
- uncompressed size: 4 bytes 
  - The size of the file compressed and uncompressed, respectively. 
  - If an archive is in zip64 format and the value in this field is 0xFFFFFFFF, the size will be in the corresponding 8 byte zip64 extended information extra field. 
  - If encrypting the central directory and general purpose bit flag 13 is set indicating masking, the value stored for the uncompressed size in the local header will be zero. 
- File name length: 2 bytes 
- Extra field length: 2 bytes 
- File name: variable bytes 
- Extra field: variable bytes 

---

#### File Data

Immediately following the local header for a file is the compressed or stored data for the file. 

The series of [local file header] [file data] [data descriptor] repeats for each file in the .ZIP archive. 

---

#### Data Descriptor

- CRC-32 
- Compressed size 
- Uncompressed size 

This descriptor exists only if bit 3 of the general purpose bit flag is set. 

It is byte aligned and immediately follows the last byte of compressed data. 

This descriptor is used only when it was not possible to seek in the output .ZIP file, e.g., when the output .ZIP file was standard output or a non seekable device. For Zip64 format archives, the compressed and uncompressed sizes are 8 bytes each. 



---

#### Archive decryption header: EFS

The Archive Decryption Header is introduced in version 6.2 of the ZIP format specification. 

<u>Refer to the section on the Strong Encryption Specification for information on the fields used in the Archive Decryption Header record.</u> 

---

#### Archive extra data record: EFS

- archive extra data signature: 4 bytes (0x08064b50) 
- extra field length: 4 bytes 
- extra field data: variable size 

This record exists in support of the Central Directory Encryption Feature implemented as part of the Strong Encryption Specification as described in this document. 

When present, this record immediately precedes the central directory data structure. 

The size of this data record will be included in the Size of the Central Directory field in the End of Central Directory record. 

If the central directory structure is compressed, but not encrypted, the location of the start of this data record is determined using the Start of Central Directory field in the Zip64 End of Central Directory record. 

---

#### Central directory structure

- [File header 1] 
- ... [File header n] 
- [digital signature] 

File header: 

- Central file header signature: 4 bytes (0x02014b50) 

- version made by: 2 bytes 

  - The upper byte indicates the compatibility of the file attribute information. 
  - If the external file attributes are compatible with MS-DOS and can be read by PKZIP for DOS version 2.04g then this value will be zero. 
  - If these attributes are not compatible, then this value will identify the host system on which the attributes are compatible. 
  - Software can use this information to determine the line record format for text files etc. Mappings are below: 
    - 0 - MS-DOS and OS/2 (FAT/VFAT/FAT32 file system) 
    - 1 - Amiga 
    - 2 - OpenVMS
    - 3 - Unix 
    - 4 - VM/CMS
    - 5 - Atari ST
    - 6 - OS/2 H.P.F.S. 
    - 7 - Macintosh 
    - 8 - Z-System 
    - 9 - CP/M 
    - 10 - Windows NTFS 
    - 11 - MVS (OS/390 - Z/OS) 
    - 12 - VSE 
    - 13 - Acorn Risc 
    - 14 - VFAT 
    - 15 - alternate MVS 
    - 16 - BeOS 
    - 17 - Tandem 
    - 18 - OS/400 
    - 19 - OS/X (Darwin) 
    - 20 - thru 255 - unused 
  - The lower byte indicates the ZIP specification version (the version of this document) supported by the software used to encode the file. 
  - The value/10 indicates the major version number, and the value mod 10 is the minor version number. 

- version needed to extract: 2 bytes 

  - The minimum supported ZIP specification version needed to extract the file, mapped as above. 
  - This value is based on the specific format features a ZIP program must support to be able to extract the file. 
  - If multiple features are applied to a file, the minimum version should be set to the feature having the highest value. 
  - New features or feature changes affecting the published format specification will be implemented using higher version numbers than the last published value to avoid conflict. 
  - Current minimum feature versions are as defined below: 
    - 1.0 - Default value 
    - 1.1 - File is a volumn label 
    - 2.0 - File is a folder (directory) 
    - 2.0 - File is compressed using Deflate compression 
    - 2.0 - File is encrypted using traditional PKWARE encryption 
    - 2.1 - File is compressed using Deflate64(tm) 
    - 2.5 - File is compressed using PKWARE DCL Implode 
    - 2.7 - File is a patch data set 
    - 4.5 - File uses ZIP64 format extensions 
    - 4.6 - File is compressed using BZIP2 compression* 
    - 5.0 - File is encrypted using DES 
    - 5.0 - File is encrypted using 3DES 
    - 5.0 - File is encrypted using original RC2 encryption 
    - 5.0 - File is encrypted using RC4 encryption 
    - 5.1 - File is encrypted using AES encryption 
    - 5.1 - File is encrypted using corrected RC2 encryption**
    - 5.2 - File is encrypted using corrected RC2-64 encryption**
    - 6.1 - File is encrypted using non-OAEP key wrapping***
    - 6.2 - Central directory encryption 
    - \* Early 7.x (pre-7.2) versions of PKZIP incorrectly set the version needed to extract for BZIP2 compression to be 50 when it should have been 46. 
    - \** Refer to the section on Strong Encryption Specification for additional information regarding RC2 corrections. 
    - \*** Certificate encryption using non-OAEP key wrapping is the intended mode of operation for all versions beginning with 6.1. Support for OAEP key wrapping should only be used for backward compatibility when sending ZIP files to be opened by versions of PKZIP older than 6.1 (5.0 or 6.0). 
    - When using ZIP64 extensions, the corresponding value in the Zip64 end of central directory record should also be set. This field currently supports only the value 45 to indicate ZIP64 extensions are present. 

- general purpose bit flag: 2 bytes 

- compression method: 2 bytes 

- last mod file time: 2 bytes 

- last mod file date: 2 bytes 

- crc-32: 4 bytes 

- compressed size: 4 bytes 

- uncompressed size: 4 bytes 

- file name length: 2 bytes 

- extra field length: 2 bytes 

- file comment length: 2 bytes 

  - The length of the file name, extra field, and comment fields respectively. 
  - The combined length of any directory record and these three fields should not generally exceed 65,535 bytes. 
  - **If input came from standard input, the file name length is set to zero.** 

- disk number start: 2 bytes 

  - The number of the disk on which this file begins. 
  - If an archive is in zip64 format, and the value in this field is 0xFFFF, the size will be in the corresponding 4 byte zip64 extended information extra field. 

- internal file attributes: 2 bytes 

  - Bits 1 and 2 are reserved for use by PKWARE. 
  - The lowest bit of this field indicates, if set, that the file is apparently an ASCII or test file. If not set, that the file apparently contains binary data. The remaining bits are unused in version 1.0 
  - The 0x0002 bit of this field indicates, if set, that a 4 byte variable record length control field precedes each logical record indicating the length of the record. This flag is independent of text control characters, and if used in conjunction with text data, includes any control characters in the total length of the record. This value is provided for mainframe data transfer support. 

- external file attributes: 4 bytes 

  - The mapping of the external attributes is host-system dependent (see 'version made by'). For MS-DOS, the low order byte is the MS-DOS directory attribute byte. 
  - If input came from standard input, this field is set to zero. 

- relative offset of local header: 4 bytes 

  - This is the offset from the start of the first disk on which this file appears, to where the local header SHOULD be found. 
  - If an archive is in ZIP64 format and the value in this field is 0xFFFFFFFF, the size will be in the corresponding 8 byte zip64 extended information extra field. 

- file name (variable size) 

  - The name of the file, with optional relative path. The path stored MUST NOT contain a drive or device letter, or a leading slash. 
  - All slashes MUST be forward slashes '/' as opposed to backwards slashes '\\' for compatibility with Amiga and UNIX file systems etc. 
  - If input came from standard input, there is not file name field. 

  ---

  - If using the Central Directory Encryption Feature and general purpose bit flag 13 is set indicating masking, the file name stored in the Local Header will not be the actual file name. 
  - A masking value consisting of a unique hexadecimal value will be stored. 
  - This value will be sequentially incremented for each file in the archive. 
  - See the section on the Strong Encryption Specification for details on retrieving the encrypted file name. 
  - Refer to the section in this document entitled "Incorporating PKWARE Proprietary Technology into Your Product" for more information. 

- extra field (variable size) 

  - This SHOULD be used for storage expansion. If additional information needs to be stored within a ZIP file for special application or platform needs, it SHOULD be stored here. 
  - Programs supporting earlier versions of this specification can then safely skip the file, and find the next file or header. 
  - This field will be 0 length in version 1.0. 
  - Existing extra fields are defined in the section. 
  - Extensible data fields that follows. 

- file comment (variable size) 

  - The comment for this file. 

---

#### Digital Signature 

- header signature: 4 bytes (0x05054b50) 
- size of data: 2 bytes 
- signature data (variable size) 

With the introduction of the Central Directory Encryption feature in version 6.2 of this specification, the Central Directory Structure may be stored both compressed and encrypted. 

Although not required, it is assumed when encrypting the Central Directory Structure, that it will be compressed for greater storage efficiency. Information on the Central Directory Encryption feature can be found in the section describing the Strong Encryption Specification. 

The digital Signature record will be neither compressed nor encrypted. 

---

#### Zip64 end of central directory record

- zip64 end of central dir signature: 4 bytes (0x06064b50) 
- size of zip64 end of central directory record: 8 bytes 
- version made by: 2 bytes 
- version needed to extract: 2 bytes 
- number of this disk: 4 bytes 
  - The number of this disk, which contains central directory end record. 
  - If an archive is in ZIP64 format and the value in this field is 0xFFFF, the size will be in the corresponding 4 byte zip64 end of central directory field. 
- number of the disk with the start of the central directory: 4 bytes 
  - The number of the disk on which the central directory starts. 
  - If an archive is in ZIP64 format and the value in this field is 0xFFFF, the size will be in the corresponding 4 byte zip64 end of central directory field. 
- total number of entries in the central directory on this disk: 8 bytes 
  - The number of central directory entries on this disk. 
  - If an archive is in ZIP64 format and the value in this field is 0xFFFF, the size will be in the corresponding 8 byte zip64 end of central directory field. 
- central directory: 8 bytes 
- size of the central directory: 8 bytes 
- offset of start of central directory with respect to the starting disk number: 8 bytes 
- zip64 extensible data sector: (variable size)
  - (currently reserved for use by PKWARE) 

The above record structure defines Version 1 of the Zip64 end of central directory record. 

Version 1 was implemented in versions of this specification preceding 6.2 in support of the ZIP64(tm) large file feature. 

The introduction of the Central Directory Encryption feature implemented in version 6.2. as part of the Strong Encryption Specification defines Version 2 of this record structure. 

Refer to the section describing the Strong Encryption Specification for details on the version 2 format for this record. 

---

#### Zip64 end of central directory locator

- Zip64 end of central dir locator signature: 4 bytes (0x07064b50) 
- number of the disk with the start of the zip64 end of central directory: 4 bytes 
- relative offset of the zip64 end of central directory record: 8 bytes 
- total number of disks: 4 bytes 

---

#### End of central directory record

- End of central dir signature: 4 bytes (0x06054b50) 
- number of this disk: 2 bytes 
- number of the disk with the start of the central directory: 2 bytes 
- total number of entries in the central directory on this disk: 2 bytes 
- total number of entries in the central directory: 2 bytes
  - The total number of files in the .ZIP file. 
  - If an archive is in ZIP64 format and the value in this field is 0xFFFF, the size will be in the corresponding 8 byte zip64 end of central directory field. 
- size of the central directory: 4 bytes 
  - The size (in bytes) of the entire central directory. 
  - If an archive is in ZIP64 format and the value in this field is 0xFFFFFFFF, the size will be in the corresponding 8 byte zip64 end of central directory field. 
- offset of start of central directory with respect to the starting disk number: 4 bytes 
  - Offset of the start of the central directory on the disk on which the central directory starts. 
  - If an archive is in ZIP64 format and the value in this field is 0xFFFFFFFF, the size will be in the corresponding 8 byte zip64 end of central directory field. 
- .ZIP file comment length: 2 bytes 
  - The length of the comment for this .ZIP file. 
- .ZIP file comment: (variable size) 
  - The comment for this .ZIP file. 
  - ZIP file comment data is stored unsecured. 
  - No encryption or data authentication is applied to this area at this time. 
  - Confidential information SHOULD NOT be stored in this section. 



### Zip Project





### Reference

- P. Deutsch DEFLATE Compressed Data Format Specification version 1.3, May 1996. https://www.ietf.org/rfc/rfc1951.txt
- PKWARE Inc., .ZIP File Format Specification 2004.4.26. https://pkware.cachefly.net/webdocs/APPNOTE/APPNOTE-6.2.0.txt


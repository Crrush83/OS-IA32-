  SECTIONS {
  main 0x100000 :
    {
    os.bin (.text)
    }
  other : {
  *(.text)
  }  

}
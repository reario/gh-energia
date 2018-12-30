int interruttore(modbus_t *m, uint16_t tab[], uint16_t reg, int bit) {
  /*
    tab[]: tabellone dei registri. 
    reg:   il registro su cui si vuole operar: contiene un numero che consideriamo come un array di bit
    bit:   all'interno della stringa di bit di tab[reg], indica su quale bit dobbiamo lavorare (cambiare stato
   */
  tab[reg]=tab[reg]^(1<<bit); // cambio stato (lo inverto rispetto a come si trova) al bit-esimo bit di tab[reg]
  if ( modbus_write_register(m,100,tab[reg]) != 1) { // lo scrivo sull'HW dell'OTB. 100 è il regitro delle uscite su OTB (Q0-Q7)
    printf("Errore WRITE registro 100 OTB");
    tab[reg]=tab[reg]^(1<<bit);      // ripristino il registro a come era prima
    printf("errore-->%i\n",errno);
    return -1;
  }
  return 0;
}

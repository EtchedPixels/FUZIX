#ifndef __SSI_H
#define __SSI_H

void ssi_init(int which);
void ssi_transfer(int port, const void *txbuffer, void *rxbuffer,
                  size_t nwords);
uint32_t ssi_setfrequency(int port, uint32_t frequency);
void ssi_rxnull(int port);
void ssi_txnull(int port);

#endif /* __SSI_H */

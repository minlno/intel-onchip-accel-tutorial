#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include <linux/idxd.h>

// TODO[OSM]
int single(uint64_t *(data_buf[][BUF_SIZE]), struct dsa_hw_desc *desc_buf,
// uint64_t* single(uint64_t *(data_buf[][BUF_SIZE]), struct dsa_hw_desc *desc_buf,
                                             struct dsa_completion_record *comp_buf,
                                             void *wq_portal) {
  int retry, status;
  uint64_t start;
  uint64_t prep = 0;
  uint64_t submit = 0;
  uint64_t wait = 0;


  // Submit 4 seprate single offloads in a row
  for (int i = 0; i < BUF_SIZE; i++) {

    ///////////////////////////////////////////////////////////////////////////
    // Descriptor Preparation
    start = rdtsc();

    // TODO[OSM]
    comp_buf[i].status          = 0;
    desc_buf[i].opcode          = DSA_OPCODE_COMPARE;
    desc_buf[i].flags           = IDXD_OP_FLAG_RCR | IDXD_OP_FLAG_CRAV;
    desc_buf[i].xfer_size       = XFER_SIZE;
    desc_buf[i].src_addr        = (uintptr_t)data_buf[0][i];
    desc_buf[i].dst_addr        = (uintptr_t)data_buf[1][i];
    desc_buf[i].completion_addr = (uintptr_t)&(comp_buf[i]);

    /*
    comp_buf[i].status          = 0;
    desc_buf[i].opcode          = DSA_OPCODE_CRCGEN;
    desc_buf[i].flags           = IDXD_OP_FLAG_RCR | IDXD_OP_FLAG_CRAV;
    desc_buf[i].xfer_size       = XFER_SIZE;
    desc_buf[i].src_addr        = (uintptr_t)data_buf[0][i];
    desc_buf[i].dst_addr        = 0;
    desc_buf[i].max_delta_size  = 0;
    desc_buf[i].completion_addr = (uintptr_t)&(comp_buf[i]);
    */

    prep += rdtsc() - start;
    ///////////////////////////////////////////////////////////////////////////



    ///////////////////////////////////////////////////////////////////////////
    // Descriptor Submission
    start = rdtsc();

    _mm_sfence();
    //movdir64b(wq_portal, &desc_buf[i]); 
    enqcmd(wq_portal, &desc_buf[i]);
    _mm_sfence();

    submit += rdtsc() - start;
    ///////////////////////////////////////////////////////////////////////////



    ///////////////////////////////////////////////////////////////////////////
    // Wait for Completion
    retry = 0;
    start = rdtsc();

    while (comp_buf[i].status == 0) {
	    ;
    }

    /*
    while (comp_buf[i].status == 0 && retry++ < MAX_COMP_RETRY) {
      umonitor(&(comp_buf[i]));
      if (comp_buf[i].status == 0) {
        uint64_t delay = __rdtsc() + UMWAIT_DELAY;
        umwait(UMWAIT_STATE_C0_1, delay);
      }
    }
    */

    wait += rdtsc() - start;
    ///////////////////////////////////////////////////////////////////////////
  }



  // TODO[OSM]
  // Print times
  printf("[time  ] preparation: %lu\n", prep);
  printf("[time  ] submission: %lu\n", submit);
  printf("[time  ] wait: %lu\n", wait);
  printf("[time  ] full offload: %lu\n", prep + submit + wait);

  status = 1;
  for (int i = 0; i < BUF_SIZE; i++) {
    if (comp_buf[i].status != 1) {
      status = comp_buf[i].status;
      break;
    }
  }

  if (status != 1)
    printf("[verify] failed: %x\n", status);
  else
    printf("[verify] passed\n");
  
  // TODO[OSM]
  return status;
  /*
  uint64_t* new = (uint64_t*)malloc(3 * sizeof(uint64_t));
  new[0] = prep;
  new[1] = submit;
  new[2] = wait;
  return new;
  */
}

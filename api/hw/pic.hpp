// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HW_PIC_HPP
#define HW_PIC_HPP

#include "../kernel/os.hpp"

class PIC {
  static const uint8_t master_ctrl = 0x20;
  static const uint8_t master_mask = 0x21; 
  static const uint8_t slave_ctrl = 0xA0;
  static const uint8_t slave_mask = 0xA1;
  
  // Master commands
  static const uint8_t master_icw1 = 0x11;
  static const uint8_t master_icw2 = 0x20;
  static const uint8_t master_icw3 = 0x04;
  static const uint8_t master_icw4 = 0x01;
  
  // Slave commands
  static const uint8_t slave_icw1 =  0x11;
  static const uint8_t slave_icw2 =  0x28;
  static const uint8_t slave_icw3 =  0x02;
  static const uint8_t slave_icw4 =  0x01;

  /* irq ready next CMD read */
  static const uint8_t read_irr = 0x0A;
  /* irq service next CMD read */
  static const uint8_t read_isr = 0x0B;

  static const uint8_t eoi_ = 0x20;

public:
  
  static void init();
  
  inline static void enable_irq(uint8_t irq){
    irq_mask_ &= ~(1 << irq);
    if (irq >= 8) irq_mask_ &= ~(1 << 2);
    set_intr_mask(irq_mask_);
    INFO2("+ Enabling IRQ %i, mask: 0x%x",irq, irq_mask_);
  };
  
  inline static void disable_irq(uint8_t irq)
  {
    irq_mask_ |= (1 << irq);
    if ((irq_mask_ & 0xFF00) == 0xFF00) 
      irq_mask_ |= (1 << 2);
    set_intr_mask(irq_mask_);
    INFO2("- Disabling IRQ %i, mask: 0x%x",irq, irq_mask_);
  }

  inline static void eoi(uint8_t irq){
    if (irq >= 8)
      OS::outb(slave_ctrl,eoi_);
    OS::outb(master_ctrl,eoi_);
  }
  
  /* Returns the combined value of the cascaded PICs irq request register */
  inline static uint16_t get_irr(void)
  {
    return get_irq_reg(read_irr);
  }
  
  /* Returns the combined value of the cascaded PICs in-service register */
  inline static uint16_t get_isr(void)
  {
    return get_irq_reg(read_isr);
  }

  
private:
  
  inline static void set_intr_mask(uint32_t mask) {
    OS::outb(master_mask, (uint8_t) mask);
    OS::outb(slave_mask, (uint8_t) (mask >> 8));
  }
  
  /* Helper func */
  inline static uint16_t get_irq_reg(int ocw3)
  {
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
   * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    OS::outb(master_ctrl, ocw3);
    OS::outb(master_ctrl, ocw3);
    return (OS::inb(slave_ctrl) << 8) | OS::inb(master_ctrl);
  }
  
  static uint16_t irq_mask_;

};

#endif

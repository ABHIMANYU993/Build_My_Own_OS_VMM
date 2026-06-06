#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#define LIMIT 0xFFFFF
#define GUEST_MEM 0x08000000
#define GUEST_CODE 0x00100000
#define INSTRUCTION 0xF4
#define GDT_ADDR 0x1000
#define CS_SELECTOR 0x08
#define DS_SELECTOR 0x10

int main(void) {
  int kvm_fd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
  if (kvm_fd < 0) {
    perror("open");
    close(kvm_fd);
    return 1;
  }
  // ask kvm version
  int version = ioctl(kvm_fd, KVM_GET_API_VERSION, 0);
  if (version < 0) {
    perror("ioctl");
    close(kvm_fd);
    return 1;
  }
  // cretae a vm
  int vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, 0);
  if (vm_fd < 0) {
    perror("ioctl");
    close(kvm_fd);
    return 1;
  }
  // Pseudo-architecture
  void *guest_ram = mmap(NULL, GUEST_MEM, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // map the guest ram to the vm
  struct kvm_userspace_memory_region region;
  region.slot = 0;
  region.flags = 0;
  region.guest_phys_addr = 0x0;
  region.memory_size = GUEST_MEM;
  region.userspace_addr = (uintptr_t)guest_ram;
  int mem_region = ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &region);
  if (mem_region < 0) {
    perror("ioctl");
    close(vm_fd);
    close(kvm_fd);
    return 1;
  }
  // vcpu creation
  int vcpu_fd = ioctl(vm_fd, KVM_CREATE_VCPU, 0);

  if (vcpu_fd < 0) {
    perror("ioctl");
    close(vm_fd);
    close(kvm_fd);
    return 1;
  }
  // reding the vcou registors
  struct kvm_sregs sregs;
  ioctl(vcpu_fd, KVM_GET_SREGS, &sregs);
  struct kvm_regs regs;
  ioctl(vcpu_fd, KVM_GET_REGS, &regs);
  printf("KVM API version: %d\nKVM_CREATE_VM:%d\nGuest_ram:%p\n"
         "KVM_CREATE_VCPU:%d\n"
         "KVM_SET_USER_MEMORY_REGION:%d\n",
         version, vm_fd, guest_ram, vcpu_fd, mem_region);

  printf("\n========== KVM SREGS DUMP(before modify) ==========\n");

  printf("CR0   = 0x%llx\n", (unsigned long long)sregs.cr0);
  printf("CR2   = 0x%llx\n", (unsigned long long)sregs.cr2);
  printf("CR3   = 0x%llx\n", (unsigned long long)sregs.cr3);
  printf("CR4   = 0x%llx\n", (unsigned long long)sregs.cr4);
  printf("EFER  = 0x%llx\n", (unsigned long long)sregs.efer);

  printf("\nGDT:\n");
  printf("  base  = 0x%llx\n", (unsigned long long)sregs.gdt.base);
  printf("  limit = 0x%x\n", sregs.gdt.limit);

  printf("\nIDT:\n");
  printf("  base  = 0x%llx\n", (unsigned long long)sregs.idt.base);
  printf("  limit = 0x%x\n", sregs.idt.limit);

  printf("\nCS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.cs.base, sregs.cs.limit, sregs.cs.selector,
         sregs.cs.type, sregs.cs.present, sregs.cs.dpl, sregs.cs.s, sregs.cs.g,
         sregs.cs.db, sregs.cs.l, sregs.cs.avl, sregs.cs.unusable);

  printf("\nDS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.ds.base, sregs.ds.limit, sregs.ds.selector,
         sregs.ds.type, sregs.ds.present, sregs.ds.dpl, sregs.ds.s, sregs.ds.g,
         sregs.ds.db, sregs.ds.l, sregs.ds.avl, sregs.ds.unusable);

  printf("\nES:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.es.base, sregs.es.limit, sregs.es.selector,
         sregs.es.type, sregs.es.present, sregs.es.dpl, sregs.es.s, sregs.es.g,
         sregs.es.db, sregs.es.l, sregs.es.avl, sregs.es.unusable);

  printf("\nFS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.fs.base, sregs.fs.limit, sregs.fs.selector,
         sregs.fs.type, sregs.fs.present, sregs.fs.dpl, sregs.fs.s, sregs.fs.g,
         sregs.fs.db, sregs.fs.l, sregs.fs.avl, sregs.fs.unusable);

  printf("\nGS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.gs.base, sregs.gs.limit, sregs.gs.selector,
         sregs.gs.type, sregs.gs.present, sregs.gs.dpl, sregs.gs.s, sregs.gs.g,
         sregs.gs.db, sregs.gs.l, sregs.gs.avl, sregs.gs.unusable);

  printf("\nSS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.ss.base, sregs.ss.limit, sregs.ss.selector,
         sregs.ss.type, sregs.ss.present, sregs.ss.dpl, sregs.ss.s, sregs.ss.g,
         sregs.ss.db, sregs.ss.l, sregs.ss.avl, sregs.ss.unusable);

  printf("\nTR:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.tr.base, sregs.tr.limit, sregs.tr.selector,
         sregs.tr.type, sregs.tr.present, sregs.tr.dpl, sregs.tr.s, sregs.tr.g,
         sregs.tr.db, sregs.tr.l, sregs.tr.avl, sregs.tr.unusable);

  printf("\nLDT:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.ldt.base, sregs.ldt.limit,
         sregs.ldt.selector, sregs.ldt.type, sregs.ldt.present, sregs.ldt.dpl,
         sregs.ldt.s, sregs.ldt.g, sregs.ldt.db, sregs.ldt.l, sregs.ldt.avl,
         sregs.ldt.unusable);

  printf("====================================\n");

  printf("\n========== KVM REGS(before modify) ==========\n");
  printf("RIP    = 0x%llx\n", (unsigned long long)regs.rip);
  printf("RSP    = 0x%llx\n", (unsigned long long)regs.rsp);
  printf("RFLAGS = 0x%llx\n", (unsigned long long)regs.rflags);

  printf("RAX = 0x%llx\n", (unsigned long long)regs.rax);
  printf("RBX = 0x%llx\n", (unsigned long long)regs.rbx);
  printf("RCX = 0x%llx\n", (unsigned long long)regs.rcx);
  printf("RDX = 0x%llx\n", (unsigned long long)regs.rdx);
  printf("RSI = 0x%llx\n", (unsigned long long)regs.rsi);
  printf("RDI = 0x%llx\n", (unsigned long long)regs.rdi);
  printf("RBP = 0x%llx\n", (unsigned long long)regs.rbp);
  printf("================================\n");

  // modifyiing sregs to make the cpu to protected mode
  // Mode Switch to protected
  sregs.cr0 |= 0x1;

  // CS(code segment) segment values for making segmentation invisible
  sregs.cs.base = 0;
  sregs.cs.g = 1;
  sregs.cs.db = 1;
  sregs.cs.limit = LIMIT;
  sregs.cs.selector = CS_SELECTOR;
  sregs.cs.type = 0xB;
  sregs.cs.s = 1;
  sregs.cs.dpl = 0;
  sregs.cs.present = 1;

  // DS(data segment) segment values to match CS
  sregs.ds.base = 0;
  sregs.ds.limit = LIMIT;
  sregs.ds.db = 1;
  sregs.ds.g = 1;
  sregs.ds.selector = DS_SELECTOR;
  sregs.ds.type = 0x3;
  sregs.ds.s = 1;
  sregs.ds.dpl = 0;
  sregs.ds.present = 1;

  // ES(extra segment, think of it like ds is src and es is destination) segment
  // values to match CS
  sregs.es.base = 0;
  sregs.es.limit = LIMIT;
  sregs.es.db = 1;
  sregs.es.g = 1;
  sregs.es.selector = DS_SELECTOR;
  sregs.es.type = 0x3;
  sregs.es.s = 1;
  sregs.es.dpl = 0;
  sregs.es.present = 1;

  // SS(stack segment) segment values to match CS
  sregs.ss.base = 0;
  sregs.ss.limit = LIMIT;
  sregs.ss.db = 1;
  sregs.ss.g = 1;
  sregs.ss.selector = DS_SELECTOR;
  sregs.ss.type = 0x3;
  sregs.ss.s = 1;
  sregs.ss.dpl = 0;
  sregs.ss.present = 1;

  // setting the unusable segments to zero for making the gp to work
  sregs.fs.unusable = 1;
  sregs.gs.unusable = 1;
  sregs.tr.unusable = 1;
  sregs.ldt.unusable = 1;

  ioctl(vcpu_fd, KVM_SET_SREGS, &sregs);
  struct kvm_sregs verify;
  ioctl(vcpu_fd, KVM_GET_SREGS, &verify);
  printf("\n========== KVM SREGS DUMP MODIFIED ==========\n");

  printf("CR0   = 0x%llx\n", (unsigned long long)sregs.cr0);
  printf("CR2   = 0x%llx\n", (unsigned long long)sregs.cr2);
  printf("CR3   = 0x%llx\n", (unsigned long long)sregs.cr3);
  printf("CR4   = 0x%llx\n", (unsigned long long)sregs.cr4);
  printf("EFER  = 0x%llx\n", (unsigned long long)sregs.efer);

  printf("\nGDT:\n");
  printf("  base  = 0x%llx\n", (unsigned long long)sregs.gdt.base);
  printf("  limit = 0x%x\n", sregs.gdt.limit);

  printf("\nIDT:\n");
  printf("  base  = 0x%llx\n", (unsigned long long)sregs.idt.base);
  printf("  limit = 0x%x\n", sregs.idt.limit);

  printf("\nCS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.cs.base, sregs.cs.limit, sregs.cs.selector,
         sregs.cs.type, sregs.cs.present, sregs.cs.dpl, sregs.cs.s, sregs.cs.g,
         sregs.cs.db, sregs.cs.l, sregs.cs.avl, sregs.cs.unusable);

  printf("\nDS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.ds.base, sregs.ds.limit, sregs.ds.selector,
         sregs.ds.type, sregs.ds.present, sregs.ds.dpl, sregs.ds.s, sregs.ds.g,
         sregs.ds.db, sregs.ds.l, sregs.ds.avl, sregs.ds.unusable);

  printf("\nES:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.es.base, sregs.es.limit, sregs.es.selector,
         sregs.es.type, sregs.es.present, sregs.es.dpl, sregs.es.s, sregs.es.g,
         sregs.es.db, sregs.es.l, sregs.es.avl, sregs.es.unusable);

  printf("\nFS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.fs.base, sregs.fs.limit, sregs.fs.selector,
         sregs.fs.type, sregs.fs.present, sregs.fs.dpl, sregs.fs.s, sregs.fs.g,
         sregs.fs.db, sregs.fs.l, sregs.fs.avl, sregs.fs.unusable);

  printf("\nGS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.gs.base, sregs.gs.limit, sregs.gs.selector,
         sregs.gs.type, sregs.gs.present, sregs.gs.dpl, sregs.gs.s, sregs.gs.g,
         sregs.gs.db, sregs.gs.l, sregs.gs.avl, sregs.gs.unusable);

  printf("\nSS:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.ss.base, sregs.ss.limit, sregs.ss.selector,
         sregs.ss.type, sregs.ss.present, sregs.ss.dpl, sregs.ss.s, sregs.ss.g,
         sregs.ss.db, sregs.ss.l, sregs.ss.avl, sregs.ss.unusable);

  printf("\nTR:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.tr.base, sregs.tr.limit, sregs.tr.selector,
         sregs.tr.type, sregs.tr.present, sregs.tr.dpl, sregs.tr.s, sregs.tr.g,
         sregs.tr.db, sregs.tr.l, sregs.tr.avl, sregs.tr.unusable);

  printf("\nLDT:\n");
  printf("  base=0x%llx limit=0x%x sel=0x%x type=0x%x p=%u dpl=%u s=%u g=%u "
         "db=%u l=%u avl=%u unusable=%u\n",
         (unsigned long long)sregs.ldt.base, sregs.ldt.limit,
         sregs.ldt.selector, sregs.ldt.type, sregs.ldt.present, sregs.ldt.dpl,
         sregs.ldt.s, sregs.ldt.g, sregs.ldt.db, sregs.ldt.l, sregs.ldt.avl,
         sregs.ldt.unusable);

  printf("====================================\n");
  // modifyiing regs to make the cpu execute guest code
  regs.rip = GUEST_CODE;
  regs.rsp = GUEST_MEM;
  ioctl(vcpu_fd, KVM_SET_REGS, &regs);
  struct kvm_regs verify_regs;
  ioctl(vcpu_fd, KVM_GET_REGS, &verify_regs);

  printf("\n========== KVM REGS MODIFIED ==========\n");
  printf("RIP    = 0x%llx\n", (unsigned long long)verify_regs.rip);
  printf("RSP    = 0x%llx\n", (unsigned long long)verify_regs.rsp);
  printf("RFLAGS = 0x%llx\n", (unsigned long long)verify_regs.rflags);

  printf("RAX = 0x%llx\n", (unsigned long long)verify_regs.rax);
  printf("RBX = 0x%llx\n", (unsigned long long)verify_regs.rbx);
  printf("RCX = 0x%llx\n", (unsigned long long)verify_regs.rcx);
  printf("RDX = 0x%llx\n", (unsigned long long)verify_regs.rdx);
  printf("RSI = 0x%llx\n", (unsigned long long)verify_regs.rsi);
  printf("RDI = 0x%llx\n", (unsigned long long)verify_regs.rdi);
  printf("RBP = 0x%llx\n", (unsigned long long)verify_regs.rbp);
  printf("================================\n");

  // guest code execution testing
  uint8_t *target_adress = (uint8_t *)guest_ram + GUEST_CODE;
  *target_adress = INSTRUCTION;
  printf("Instruction at Guest_phys_add(%p):%x\n", target_adress, INSTRUCTION);

  // shared memmory setup for KVM and USERSPACE so low latency and compute is
  // saved for large scale instruction
  int kvm_shared_space_size = ioctl(kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
  printf("KVM vCPU mmap size =%d\n", kvm_shared_space_size);

  // mmap the shared mem for kvm and userspace
  struct kvm_run *run = mmap(NULL, kvm_shared_space_size,
                             PROT_READ | PROT_WRITE, MAP_SHARED, vcpu_fd, 0);

  int ret = ioctl(vcpu_fd, KVM_RUN, 0);
  printf("ret = %d\n", ret);
  struct kvm_regs after;
  ioctl(vcpu_fd, KVM_GET_REGS, &after);

  printf("RIP after = 0x%llx\n", after.rip);
  printf("Exit reason = %u\n", run->exit_reason);
  printf("IO.port      = 0x%x\n", run->io.port);
  printf("IO.direction = %u\n", run->io.direction);
  printf("IO.size      = %u\n", run->io.size);
  printf("IO.count     = %u\n", run->io.count);

  struct kvm_vcpu_events events;
  ioctl(vcpu_fd, KVM_GET_VCPU_EVENTS, &events);

  printf("exception.injected = %u\n", events.exception.injected);
  printf("exception.nr       = %u\n", events.exception.nr);
  printf("exception.has_error_code = %u\n", events.exception.has_error_code);
  printf("exception.error_code     = 0x%x\n", events.exception.error_code);
  close(kvm_fd);
  return 0;
}

#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#define LIMIT 0xFFFFF
#define GUEST_MEM 0x08000000
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
  int ret = ioctl(vcpu_fd, KVM_GET_SREGS, &sregs);
  struct kvm_regs regs;
  ioctl(vcpu_fd, KVM_GET_REGS, &regs);
  printf("KVM API version: %d\nKVM_CREATE_VM:%d\nGuest_ram:%p\n"
         "KVM_CREATE_VCPU:%d\n"
         "KVM_SET_USER_MEMORY_REGION:%d\n",
         version, vm_fd, guest_ram, vcpu_fd, mem_region);
  printf("CR0      = 0x%llx\n", sregs.cr0);

  printf("CS.base  = 0x%llx\n", sregs.cs.base);
  printf("CS.limit = 0x%x\n", sregs.cs.limit);
  printf("CS.sel   = 0x%x\n", sregs.cs.selector);

  printf("DS.base  = 0x%llx\n", sregs.ds.base);
  printf("DS.limit = 0x%x\n", sregs.ds.limit);
  printf("DS.sel   = 0x%x\n", sregs.ds.selector);
  printf("RIP = 0x%llx\n", regs.rip);
  printf("RFLAGS = 0x%llx\n", regs.rflags);

  printf("CR3 = 0x%llx\n", sregs.cr3);
  printf("CR4 = 0x%llx\n", sregs.cr4);
  printf("EFER = 0x%llx\n", sregs.efer);

  printf("CS.type     = 0x%x\n", sregs.cs.type);
  printf("CS.present  = %d\n", sregs.cs.present);
  printf("CS.db       = %d\n", sregs.cs.db);
  printf("CS.g        = %d\n", sregs.cs.g);
  printf("DS.type     = 0x%x\n", sregs.ds.type);
  printf("DS.present  = %d\n", sregs.ds.present);
  printf("DS.db       = %d\n", sregs.ds.db);
  printf("DS.g        = %d\n", sregs.ds.g);

  // modifyiing sregs to make the cpu to protected mode
  // Mode Switch to protected
  sregs.cr0 |= 0x1;

  // CS(code segment) segment values for making segmentation invisible
  sregs.cs.base = 0;
  sregs.cs.g = 1;
  sregs.cs.db = 1;
  sregs.cs.limit = LIMIT;

  // DS(data segment) segment values to match CS
  sregs.ds.base = 0;
  sregs.ds.limit = LIMIT;
  sregs.ds.g = 1;

  // ES(extra segment, think of it like ds is src and es is destination) segment
  // values to match CS
  sregs.es.base = 0;
  sregs.es.limit = LIMIT;
  sregs.es.g = 1;

  // SS(stack segment) segment values to match CS
  sregs.ss.base = 0;
  sregs.ss.limit = LIMIT;
  sregs.ss.g = 1;
  ioctl(vcpu_fd, KVM_SET_SREGS, &sregs);
  struct kvm_sregs verify;
  ioctl(vcpu_fd, KVM_GET_SREGS, &verify);
  printf("===============modified SREGS and REGS===============\n");
  printf("CR0      = 0x%llx\n", verify.cr0);
  printf("CS.base  = 0x%llx\n", verify.cs.base);
  printf("CS.limit = 0x%x\n", verify.cs.limit);
  printf("CS.g     = %d\n", verify.cs.g);
  printf("CS.db    = %d\n", verify.cs.db);
  // debugging statements
  // printf("ES.base= 0x%llx\n", sregs.es.base);
  // printf("FS.base= 0x%llx\n", sregs.fs.base);
  // printf("GS.base= 0x%llx\n", sregs.gs.base);
  // printf("SS.base= 0x%llx\n", sregs.ss.base);
  // printf("ES.sel = 0x%x\n", sregs.es.selector);
  // printf("FS.sel = 0x%x\n", sregs.fs.selector);
  // printf("GS.sel = 0x%x\n", sregs.gs.selector);
  // printf("SS.sel = 0x%x\n", sregs.ss.selector);

  printf("DS.base  = 0x%llx\n", verify.ds.base);
  printf("DS.limit = 0x%x\n", verify.ds.limit);
  printf("DS.g     = %d\n", verify.ds.g);
  printf("ES.base  = 0x%llx\n", verify.es.base);
  printf("ES.limit = 0x%x\n", verify.es.limit);
  printf("ES.g     = %d\n", verify.es.g);
  printf("SS.base  = 0x%llx\n", verify.ss.base);
  printf("SS.limit = 0x%x\n", verify.ss.limit);
  printf("SS.g     = %d\n", verify.ss.g);
  close(kvm_fd);
  return 0;
}

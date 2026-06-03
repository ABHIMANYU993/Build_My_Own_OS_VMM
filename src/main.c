#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

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
  void *guest_ram = mmap(NULL, 0x08000000, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // map the guest ram to the vm
  struct kvm_userspace_memory_region region;
  region.slot = 0;
  region.flags = 0;
  region.guest_phys_addr = 0x0;
  region.memory_size = 0x08000000;
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
  printf("KVM API version: %d\nKVM_CREATE_VM:%d\nGuest_ram:%p\n"
         "KVM_CREATE_VCPU:%d\n"
         "KVM_SET_USER_MEMORY_REGION:%d\n",
         version, vm_fd, guest_ram, vcpu_fd, mem_region);

  close(kvm_fd);
  return 0;
}

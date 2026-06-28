# My Homemade VMM & OS Playground 🚀

Hey there! Welcome to my low-level hands-on lab. 

## So... Why does this repository exist? 🤔
I was working on an agentic OS project and realized I didn't actually know what an operating system *means* under the hood. So I kept that project on hold. I wanted to boot and run an OS on bare metal, but I didn't have a spare physical CPU lying around. 

Instead of just running a prebuilt VM (like QEMU or VirtualBox) where all the magic is hidden from me, I decided to build my own bare-metal **Virtual Machine Monitor (VMM)** from scratch using the Linux **KVM (Kernel-based Virtual Machine)** API! 

This repository is my journey of learning the flow—from raw silicon up to loading a bootloader.

---

## How we did it (The Commit-by-Commit Chronicles) 🛠️

Here is how the project evolved, straight from the git commit logs and debug sessions:

1. **Talking to the Kernel (`started the custom vmm build`)**
   First, we opened `/dev/kvm`, asked for the API version, and initialized a virtual machine. We allocated `128MB` of page-aligned guest memory using `mmap`, mapped it with `KVM_SET_USER_MEMORY_REGION`, and spawned a vCPU.

2. **Wrestling with Segment Registers (`CS_PE`, `G and Limit`, `DS ES SS flattened`)**
   Modern CPUs boot in 16-bit Real Mode. We wanted 32-bit Protected Mode. So we enabled the protected mode bit in `CR0` and configured the segmentation registers (`cs`, `ds`, `es`, `ss`). To keep memory addressing linear and simple, we flattened the segments to cover the entire 4GB address space (`limit = 0xFFFFFFFF`) and enabled page granularity.

3. **Debugging the Triple Faults 💥 (`solving triple fault from gdt and GP`)**
   Oh boy, triple faults! We got hit by invalid guest states because of strict Intel hardware rules. We learned that even if registers like task register (`tr`), `ldt`, `fs`, and `gs` are unused, they *cannot* just be random null values—we had to set their `unusable` flags to `1` (or configure them correctly) so the processor doesn't crash on boot.

4. **Running Our First Instructions (`executed the 9 byte instructions`)**
   We loaded an 8-byte guest code payload into the guest's RAM using `memcpy`:
   ```assembly
   mov dx, 0x3F8   ; COM1 port
   mov al, 'A'     ; Our ASCII char
   out dx, al      ; Send it!
   hlt             ; Halt the CPU
   ```
   Our VMM loop captures the VM exit, reads the data from the COM1 port offset, prints `'A'` to the terminal, and cleanly stops on `HLT`. No prebuilt VM, just raw bytes executing on the physical CPU!

5. **Building a Proper OS Boot Environment (`set environment for OS Boot.asm, Linker.ld`)**
   We wrote a simple assembly bootloader (`boot.asm`) and a custom linker script (`linker.ld`). Through this, we learned why the linker is so critical—it is the only tool that resolves symbolic names into absolute physical memory addresses that the CPU can jump to.

---

## How to Run It 💻

### Prerequisites
You need a Linux system with KVM enabled and write access to `/dev/kvm`. You also need `clang`, `nasm`, and `make`.

### Run the VMM
Navigate to the `vmm/` directory and compile/run the host monitor:
```bash
cd vmm
make run
```
This will compile the C monitor, initialize the VM, load the guest payload, and run it. You should see the registers dump and the output character from the VM!

### Build the OS
Navigate to the `os/` directory to build the 32-bit guest OS kernel:
```bash
cd os
make
```

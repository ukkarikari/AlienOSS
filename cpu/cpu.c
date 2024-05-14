#include "cpu.h"
#include "../list/list.h"
#include "../scheduler/scheduler.h"

/// Necessary for global variables
extern Scheduler *scheduler;
extern sem_t process_semaphore;
extern Memory *memory;
extern List *PCB;
void init_cpu() {
  pthread_t cpu_id;

  pthread_attr_t cpu_attr;

  pthread_attr_init(&cpu_attr);

  pthread_attr_setscope(&cpu_attr, PTHREAD_SCOPE_SYSTEM);

  pthread_create(&cpu_id, NULL, (void *)execute, NULL);
}

void cpu() {
  while (1) {                          /// constantly running
    if (!scheduler->running_process) { /// no running process
      forward_scheduling();            /// schedules another process
    } else {                           /// there is a scheduled process already
      while (scheduler->running_process &&
             scheduler->running_process->PC <
                 scheduler->running_process->segment->num_instructions &&
             scheduler->running_process->remaining_time >
                 0) { /// while the process is still running

        sem_wait(&process_semaphore);

        Process *running = scheduler->running_process;

        if (!running->segment
                 ->present_bit) { // segment is not loaded in the memory
          // chamar função de carregar, que chama interrupção e tira o processo
          // de executar
        } else {
          int page_number = fetch_instruction(running->PC);
          // procura página na tabela de páginas e muda bit

          process_instruction(
              running,
              running->segment
                  ->instructions[running->PC]); // aumenta PC aqui se der certo
        }

        sem_post(&process_semaphore);
      }

      /// exited while, that means the process stopped running, either because
      /// it was interrupted, because it has finished running, or because it
      /// used all of its quantum time

      if (scheduler->running_process->PC >=
          scheduler->running_process->segment->num_instructions) { /// finished

        process_finish_syscall(); // process finished
      }

      if (scheduler->running_process->remaining_time <=
          0) { // completed the quantum time
        process_interrupt_syscall();
      }
    }
  }
}

int fetch_instruction(int PC) {
  return PC / PAGE_SIZE; /// the number of the page where the instruction is
}

void process_instruction(Process *process, Instruction instruction) {
  switch (instruction.opcode) {
  /// EXEC has the format EXEC X, where X is the necessary time to execute  a
  /// given instruction
  case EXEC:
    process->remaining_time -= instruction.operand;
    /// updates program counter
    process->PC++;
    break;

    /// later
  case READ:
    break;

  /// later
  case WRITE:
    break;

  case P:
    char semaphore_id = instruction.semaphore;
    Node *semaphore_node = find(memory->semaphores, &semaphore_id);

    Semaphore *semaphore = (Semaphore *)semaphore_node;
    semaphore_p_syscall(semaphore);

    break;

  case V:

    char semaphore_id = instruction.semaphore;
    Node *semaphore_node = find(memory->semaphores, &semaphore_id);

    Semaphore *semaphore = (Semaphore *)semaphore_node;
    semaphore_v_syscall(semaphore);

    break;

  /// later
  case PRINT:
    break;

  default:
    printf("Error: the opcode is invalid");
    exit(0);
  }
}

void process_interrupt_syscall() {
  scheduler->running_process->status = WAITING;

  /// re-schedules
  forward_scheduling();
}

void semaphore_p_syscall(Semaphore *semaphore) {
  /// if there are no elements in the semaphore's queue
  if (semaphore->owner_id == -1) {
    semaphore->owner_id = scheduler->running_process->id;
    scheduler->running_process->remaining_time -=
        200; /// 200 time required to semaphore
  }
  /// enqueue the process if there are elements in the queue
  else {
    push(semaphore->processes_waiting, scheduler->running_process);
    scheduler->running_process->status = WAITING;

    /// forwards the scheduling to choose the next process and remove the
    /// current one
    forward_scheduling();
  }
}
void semaphore_v_syscall(Semaphore *semaphore) {
  semaphore->owner_id = -1;

  Process *new_process = (Process *)pop(semaphore->processes_waiting);

  if (new_process) {
    new_process->status = READY;

    add_process_scheduler(new_process);

    semaphore->owner_id = new_process->id;
  }

  forward_scheduling();
}

void process_finish_syscall() {
  /// delete the scheduler from the PCB
  delete_list(PCB, scheduler->running_process);

  /// calls the scheduler to advance scheduling
  forward_scheduling();
}

void memory_load_syscall() {
  /// calls here function to load the memory
  /// func
  ///

  /// change process status and calls forward_scheduling to remove it
  /// and schedule the next running process
  scheduler->running_process->status = WAITING;
  forward_schedulin();
}

void process_create_syscall() {

  /// @TEMP não entendi se a gente vai passar o nome do arquivo aqui
  /// e chamar a create process sem arquivo ou se nem chama a create process

  /// the last created process will be on tail
  push(scheduler->ready_processes, PCB->tail);

  /// creating a new process interrupts the current one.
  process_interrupt_syscall();
}

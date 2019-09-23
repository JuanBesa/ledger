#include "dmlf/vm_wrapper_systemcommand.hpp"

#include <unistd.h>


namespace fetch {
namespace dmlf {




void VmWrapperSystemcommand::Execute(std::string /*entrypoint*/, const Params /*params*/)
{
  ::pipe(stdin_pipe); // 0 = read end, 1 = write end.
  ::pipe(stderr_pipe);
  ::pipe(stdout_pipe);

  // stdin is the other way round.
  std::swap(stdin_pipe[0], stdin_pipe[1]);

  // Now, ZERO contains the server end, ONE contains the child end.

  int p = ::fork();
  switch(p)
  {
  case 0:
    //we are the child.
  case -1:
    break;
    // error
  default:
    break;
    // we are the parent.
    
  }
}

void VmWrapperSystemcommand::SetStdout(OutputHandler)
{
}

void VmWrapperSystemcommand::SetStdin(InputHandler)
{
}

void VmWrapperSystemcommand::SetStderr(OutputHandler)
{
}



}
}

/*
תרגיל מס' 1
שלום שטיינבך - 327161444
שניר חדד - 208938357
קבוצת תרגול - 01
*/
<?hh
function main() {
  $srcDir =
    readline("Enter the source directory with vm code to be compiled:  ");
  $srcDir = getcwd().'/'.trim($srcDir); # getcwd == 'get current working directory'
  $dstDir = readline("Enter the destination directory:  ");
  $dstDir = getcwd().'/'.trim($dstDir);
  if (!is_dir($dstDir))
    mkdir($dstDir);
  $paths = scandir($srcDir);
  array_splice($paths, 0, 2); # the first two elements in the array are: '.', '..' - these are unneeded

  foreach ($paths as $key) {
    if (pathinfo($key, PATHINFO_EXTENSION) !== 'vm') # checks if the file has extension other than 'vm'. If so, skip over
      continue;

    echo "\nWorking on ".$key."...\n";

    $srcFile = fopen("$srcDir/$key", 'r');
    $dstFile = fopen("$dstDir/".strtok($key, '.').".asm", 'w'); # creating dest file with same name but with extension 'asm'

    while (!feof($srcFile)) {
      $line = fgets($srcFile);
      if (substr($line, 0, 2) === '//' || feof($srcFile)) { # if the line is a comment, skip it
        continue;
      } else {
        fwrite($dstFile, compile($line)); # write the compiled line to the dest file
      }
    }
    fclose($srcFile);
    fclose($dstFile);
  }
}

/*
 * This function takes a line of VM code, processes it, and outputs the equivalent Hack code.
 */
function compile(string $line): string {
  $command = explode(' ', $line, 5); # convert line into an array of literals
  $retString = '';
  switch (trim($command[0])) {
    # the 'trim' solves compatibility issues for files written in Windows
    case "add":
      $retString = binary('+');
      break;
    case "sub":
      $retString = binary('-');
      break;
    case "neg":
      $retString = unary('-');
      break;
    case "eq":
      $retString = comparison('EQ');
      break;
    case "gt":
      $retString = comparison('GT');
      break;
    case "lt":
      $retString = comparison('LT');
      break;
    case "and":
      $retString = binary('&');
      break;
    case "or":
      $retString = binary('|');
      break;
    case "not":
      $retString = unary('!');
      break;
    case "push":
      $retString = push($command[1], $command[2]);
      break;
    case "pop":
      $retString = pop($command[1], $command[2]);
      break;
    default:
  }

  return $retString;
}

/*
 * Generic 'push' function. Deals with all cases of 'push segment offset'.
 */
function push(string $segment, string $offset): string {
  if ($segment === "constant") { # 'constant' is a special case
    return pushConstant($offset);
  }

  # $loadBasePointer will be the segment base pointer
  # $loadAddress will be the method we use to calculate segment[offset]
  switch ($segment) {
    case "argument":
      $loadBasePointer = "ARG";
      $loadAddress = "A=M+D";
      break;
    case "local":
      $loadBasePointer = "LCL";
      $loadAddress = "A=M+D";
      break;
    case "static":
      $loadBasePointer = "16";
      $loadAddress = "A=A+D";
      break;
    case "this":
      $loadBasePointer = "THIS";
      $loadAddress = "A=M+D";
      break;
    case "that":
      $loadBasePointer = "THAT";
      $loadAddress = "A=M+D";
      break;
    case "pointer":
      $loadBasePointer = "3";
      $loadAddress = "A=A+D";
      break;
    case "temp":
      $loadBasePointer = "5";
      $loadAddress = "A=A+D";
      break;
    default:
      return '';
  }

  $retString = "@$offset"; # load $offset into A
  $retString .= "D=A\n";
  $retString .= "@$loadBasePointer\n"; # load the segment base pointer into A
  $retString .= "$loadAddress\n"; # get the address for 'base pointer + offset'. (In other words the address of segment[offset])
  $retString .= "D=M\n"; # load value of segment[offset] into D
  $retString .= pushRegD(); # push value in D (segment[offset]) onto the Stack

  return $retString;
}

/*
 * Pushes a constant value onto the Stack.
 */
function pushConstant(string $constVal): string {
  $retString = "@$constVal"; # A = $constVal
  $retString .= "D=A\n"; # D = $constVal
  $retString .= pushRegD(); # pushes register D onto the Stack

  return $retString;
}

/*
 * Pushes value of register D onto the Stack.
 * WARNING! Function changes the value of register A!
 */
function pushRegD(): string {
  $retString = "@SP\n";
  $retString .= "A=M\n"; # A = address of top of Stack
  $retString .= "M=D\n"; # top of Stack = D ($constVal)
  $retString .= "@SP\n";
  $retString .= "M=M+1\n"; # increment SP by 1

  return $retString;
}

/*
 * Pops the top of the Stack into segment[offset].
 */
function pop(string $segment, string $offset): string {
  # $loadBasePointer will be the segment base pointer
  # $loadAddress will be the method we use to calculate segment[offset]
  switch ($segment) {
    case "argument":
      $loadBasePointer = "ARG";
      $loadAddress = "D=M+D";
      break;
    case "local":
      $loadBasePointer = "LCL";
      $loadAddress = "D=M+D";
      break;
    case "static":
      $loadBasePointer = "16";
      $loadAddress = "D=A+D";
      break;
    case "this":
      $loadBasePointer = "THIS";
      $loadAddress = "D=M+D";
      break;
    case "that":
      $loadBasePointer = "THAT";
      $loadAddress = "D=M+D";
      break;
    case "pointer":
      $loadBasePointer = "3";
      $loadAddress = "D=A+D";
      break;
    case "temp":
      $loadBasePointer = "5";
      $loadAddress = "D=A+D";
      break;
    default:
      return '';
  }
  $retString = "@SP\n";
  $retString .= "M=M-1\n";
  $retString .= "@$offset\n";
  $retString .= "D=A\n";
  $retString .= "@$loadBasePointer\n";
  $retString .= "$loadAddress\n";
  $retString .= "@R13\n";
  $retString .= "M=D\n";
  $retString .= "@SP\n";
  $retString .= "A=M\n";
  $retString .= "D=M\n";
  $retString .= "@R13\n";
  $retString .= "A=M\n";
  $retString .= "M=D\n";

  return $retString;
}

/*
 * Pops the top of the Stack into the given register.
 * WARNING! Function changes the value of register A always!
 */
function popToReg(string $reg): string {
  $retString = "@SP\n";
  $retString .= "M=M-1\n";
  $retString .= "A=M\n";
  $retString .= "$reg=M\n";

  return $retString;
}

/*
 * Generic unary operator. Parameter 'op' can have the values: '!' or '-'
 */
function unary(string $op): string {
  $retString = popToReg('D');
  $retString .= "D=$op"."D\n";
  $retString .= pushRegD();

  return $retString;
}

/*
 * Generic binary operator. Parameter 'op' can have the values: '+', '-', '&', or '|'.
 */
function binary(string $op): string {
  $retString = popToReg('D');
  $retString .= popToReg('A');
  $retString .= "D=A$op"."D\n";
  $retString .= pushRegD();

  return $retString;
}

/*
 * Generic comparison operator. Parameter 'op' can have the values: 'EQ', 'GT', or 'LT'
 */
function comparison(string $op): string {
  static $counter = 0;
  $retString = popToReg('D');
  $retString .= popToReg('A');
  $retString .= "D=A-D\n";
  $retString .= "@$op$counter\n";
  $retString .= "D;J$op\n";
  $retString .= "@NOT_$op$counter\n";
  $retString .= "D=0;JMP\n";
  $retString .= "($op$counter)\n";
  $retString .= "D=-1\n";
  $retString .= "(NOT_$op$counter)\n";
  $retString .= pushRegD();

  $counter++;

  return $retString;
}

main();

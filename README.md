# Simple CNC Emulation

* This is a simple implementation of a CNC machine that has $2$ stepper motors, and receives commands using the UART communication protocol.

* The commands are in the form of $2$ bytes representing the number of steps to move in the $X$ and $Y$ directions, respectively. The motors should start and finish at the same time, so any line can be drawn by only specifying those $2$ bytes, and hence any shape can be drawn.

* The commands can also be sent while the CNC is working, as it receives and stores them in time, so a list of commands can be sent in order pragmatically with specific delays between commands.

* The motors should work with a maximum speed of $300\ RPM$ and have $200$ steps per revolution. The commands are sent to them using the SPI communication protocol.

## Design Criterion

* Firstly, we need to calculate the delay between each $2$ steps of the motor:

$$ 300 \frac{revs}{min.} = 300 \times 200 \frac{steps}{min.} = \frac{300 \times 200}{60} \frac{steps}{sec.} = 1000 \frac{steps}{sec.} $$

* Therefore, we need a minimum of $1\ ms$ delay between steps.

* Secondly, to make sure that lines other than vertical and horizontal lines can be drawn, the motors should operate as specified below:

$$ \frac{\text{Completed steps in X direction}}{\text{Required steps in Y direction}} \approx \frac{\text{Completed steps in Y direction}}{\text{Required steps in Y direction}} $$

* Since it is impossible to achieve this all the time, the following equation is used to determine which motor will move a single step:

$$ \text{completed}\textunderscore\text{x}\textunderscore\text{moves} \times \text{required}\textunderscore\text{y}\textunderscore\text{moves} \lt \text{completed}\textunderscore\text{y}\textunderscore\text{moves} \times \text{required}\textunderscore\text{x}\textunderscore\text{moves} $$

* If this condition is true, the motor in the X direction should move next, and vice versa.

## Code Design

* To achieve responsiveness in the UART communication, an ISR is needed to capture the input when sent immediately and store it somewhere till the motors finish previous commands.

* To make sure nothing goes wrong because the ISR is writing to a variable that is used somewhere else, a proper synchronization is needed when the machine fetches the next command from the buffer, and this is achieved by disabling interrupts and enabling them again later.

- Note: The code is written for the CCSC compiler

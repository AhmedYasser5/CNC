#include <16F877A.h>

#fuses NOWDT

#use delay(clock = 20000000)
#use rs232(UART1, baud = 9600)

#define CAPACITY 50
typedef struct {
    int8 next_byte_index;
    int8 size;
    int8 bytes[CAPACITY];
} buf_Buffer;

void buf_init(buf_Buffer* self);

int8 buf_pop(buf_Buffer* self, int8* returned_byte);

int8 buf_blocking_pop(buf_Buffer* self);

int8 buf_push(buf_Buffer* self, int8 pushed_byte);

buf_Buffer queue;

#INT_RDA
void byte_uart_capture() { buf_push(&queue, getc()); }

typedef struct {
    int8 x_motor : 4;
    int8 y_motor : 4;
} stepper_XYMotors;

void stepper_init(stepper_XYMotors* self);

void stepper_move_x_motor(stepper_XYMotors* self);

void stepper_move_y_motor(stepper_XYMotors* self);

void main() {
    stepper_XYMotors xy_motors;
    stepper_init(&xy_motors);
    setup_spi(SPI_MASTER);
    buf_init(&queue);
    enable_interrupts(INT_RDA);
    enable_interrupts(GLOBAL);
    while (1) {
        int16 required_x_moves = buf_blocking_pop(&queue);
        int16 required_y_moves = buf_blocking_pop(&queue);
        int16 completed_x_moves = 0;
        int16 completed_y_moves = 0;
        while (completed_x_moves < required_x_moves ||
               completed_y_moves < required_y_moves) {
            if (completed_x_moves * required_y_moves <
                completed_y_moves * required_x_moves) {
                stepper_move_x_motor(&xy_motors);
                completed_x_moves++;
            } else {
                stepper_move_y_motor(&xy_motors);
                completed_y_moves++;
            }
        }
    }
}

void buf_init(buf_Buffer* self) {
    self->next_byte_index = 0;
    self->size = 0;
}

int8 buf_pop(buf_Buffer* self, int8* returned_byte) {
    if (self->size == 0) {
        return -1;
    }
    *returned_byte = self->bytes[self->next_byte_index];
    self->next_byte_index++;
    if (self->next_byte_index == CAPACITY) {
        self->next_byte_index = 0;
    }
    self->size--;
    return 0;
}

int8 buf_blocking_pop(buf_Buffer* self) {
    int8 returned_byte;
    while (self->size == 0)
        ;
    disable_interrupts(GLOBAL);
    buf_pop(self, &returned_byte);
    enable_interrupts(GLOBAL);
    return returned_byte;
}

int8 buf_push(buf_Buffer* self, int8 pushed_byte) {
    if (self->size == CAPACITY) {
        return -1;
    }
    int8 last_index = self->next_byte_index + self->size;
    if (last_index >= CAPACITY) {
        last_index -= CAPACITY;
    }
    self->bytes[last_index] = pushed_byte;
    self->size++;
    return 0;
}

void stepper_init(stepper_XYMotors* self) {
    self->x_motor = 1;
    self->y_motor = 1;
}

void stepper_send_spi_xy_motors(stepper_XYMotors* self) {
    spi_write(*(char*)self);
    delay_ms(1);
}

void stepper_move_x_motor(stepper_XYMotors* self) {
    if (self->x_motor == 8) {
        self->x_motor = 1;
    } else {
        self->x_motor <<= 1;
    }
    stepper_send_spi_xy_motors(self);
}

void stepper_move_y_motor(stepper_XYMotors* self) {
    if (self->y_motor == 8) {
        self->y_motor = 1;
    } else {
        self->y_motor <<= 1;
    }
    stepper_send_spi_xy_motors(self);
}

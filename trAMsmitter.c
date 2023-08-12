//#define F_CPU 1000000L
#define F_CPU 8000000L

#include <avr/interrupt.h>
#include <util/delay.h>

//#define FREQ_KHZ 166
#define FREQ_KHZ 220
#define FREQ_HZ (FREQ_KHZ * 1000L)

#define PIN_AM_OUTPUT PB1
#define PIN_AUDIO_INPUT PB4

#define CarrierOn() DDRB |= _BV(PIN_AM_OUTPUT)
#define CarrierOff() DDRB &= ~_BV(PIN_AM_OUTPUT)
#define SetFrequency(f) OCR1C = f
#define SetAudioOut(a) OCR0A = a


ISR(TIM0_OVF_vect) {
  SetAudioOut(ADCH) ;
  CarrierOn() ;
}


ISR(TIM0_COMPA_vect) {
  CarrierOff() ;
}


void StartCarrier(void) {
  CarrierOn() ;

  TCCR1 = \
    // Clear on compare match
    (1<<CTC1) | \
    // Toggle OC1A output line
    ((0<<COM1A1)|(1<<COM1A0)) | \
    // No prescaling
    ((0<<CS13)|(0<<CS12)|(0<<CS11)|(1<<CS10)) ;

  GTCCR = 0 ;
  SetFrequency((uint8_t) (F_CPU / (FREQ_HZ * 2) - 1)) ;
}


void StartModulation(void) {
  TCCR0A = \
    // OC0A disconnected
    ((0<<COM0A1)|(0<<COM0A0)) | \
    // 0COB disconnected
    ((0<<COM0B1)|(0<<COM0B0)) | \
    // Fast PWM
    ((1<<WGM01)|(1<<WGM00)) ;

  TCCR0B = \
    // Fast PWM OCRA
    (1<<WGM02) | \
    // No prescaling
    ((0<<CS02)|(0<<CS01)|(1<<CS00)) ;

  TIMSK = \
    // Output compare match A interrupt enable
    ((1<<OCIE0A)|(0<<OCIE0B)) | \
    // Overflow interrupt enable
    (1<<TOIE0) ;
}


void SetAudioIn() {
  // ADC2 = PB4
  ADMUX = ((0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0)) ;
  // 1.1V reference
  ADMUX |= ((0<<REFS2)|(1<<REFS1)|(0<<REFS0)) ;
  // Left adjust
  ADMUX |= (1<<ADLAR) ;

  // Enable ADC
  ADCSRA = (1<<ADEN) ;
  // Division 128
  ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)) ;
  // Auto trigger
  ADCSRA |= (1<<ADATE) ;

  // Free running
  ADCSRB = ((0<<ADTS2)|(0<<ADTS1)|(0<<ADTS0)) ;

  // Digit input disable ADC2
  DIDR0 = (1<<ADC2D) ;

  // Start conversion
  ADCSRA |= (1<<ADSC) ;
}


int main(void) {

  SetAudioIn() ;
  StartCarrier() ;
  StartModulation() ;
  sei() ;

  for(;;) ;

}

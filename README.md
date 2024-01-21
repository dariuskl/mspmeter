# MSPmeter

This is an ongoing development for a flexible voltage & current meter based on
MSP430i2x, currently in somewhat of a proof-of-concept phase. The goal is to
have a template that can be easily adapted to replace analog or inaccurate
digital panel meters in existing power supplies or to integrate into new
designs. Accuracy-wise it would be nice to achieve .1 mV resp. .1 mA, so that
the readout can be trusted at a resolution of 1 mV resp. 1 mA.

## Planned features

- Buzzer
- Persistent calibration

## User manual

### Digital readout & indicators

### Heartbeat LED

### Buzzer

### Serial interface

9600N1

### Calibration & configuration


# Hardware

## Design notes

### Achievable accuracy

The required accuracy is 1 mV maximum error for the voltage measurement (because
we are aiming for a resolution of 10 mV) and 0.1 mA maximum error for the
current measurement (again, because we are aiming for a resolution of 1 mA).

The internal measurement circuitry of the power supply sources 1 mA for
full-scale voltage and current. To obtain 928 mV max. full-scale ADC input,
a resistor is used to drop a proportional voltage.

    R = 928 mV / 1 mA = 928 Ω

The next closest standard value *below* (we do not want to exceed that voltage)
is 910 Ω. The best readily available performer is `RN73H1JTTD9100A05` with 0.05%
tolerance and 5ppm/°C temperature drift.

That configuration yields below voltage input specification to the ADC.

    0..1 mA / (910 R ± 0.05% ± 5ppm/°C) = 910 mV ± 0.05% ± 5ppm/°C

An operating temperature range of 15 to 60 °C is a reasonable choice for such an
instrument. This yields a worst-case error range of

    0.910 * 0.000005 * (15 - 25) = −0,0000455 (~0,05 mV)
    0.910 * 0.000005 * (60 - 25) =  0,00015925 (~0,16 mV)

The final accuracy specification based on the input circuit over the full
temperature range is thus ~ 5 mV error (in terms of measured output voltage)
and ~ 0.2 mA error (in terms of measured output current).

## I/O

    ──────────────┐
             P1.0 ├─ HEARTBEAT
             P1.1 ├─
     UCA0RXD/P1.2 ├─ Rx
     UCA0TXD/P1.3 ├─ Tx
     UCB0STE/P1.4 ├─ RCLK
     UCB0CLK/P1.5 ├─ SRCLK
       TA0.2/P1.6 ├─ BUZZER
    UCB0SIMO/P1.7 ├─ SER
                  │
             P2.0 ├─ nS1_PRESSED
             P2.1 ├─ ENCODER_A
             P2.2 ├─ ENCODER_B
             P2.3 ├─
    ──────────────┘

## Adjustment procedure

A correctly adjusted unit can achieve accuracy of ???

| Parameter | VCC | Min | Typ | Max | Unit |
|-----------|-----|-----|-----|-----|------|
| VREF      | 3 V | 1.0 | 1.2 | 1.5 | V    |
| ZID       | 3 V | 300 | 400 |     | kΩ   |

The adjustment steps have to performed in the exact order in which they are
given here and no steps may be skipped. There is no way to cancel the adjustment
half-way through.

### Input attenuator trim

If the input attenuator consists of fixed resistors, this step will be skipped.
It is not advisable to use a trimmer due to the associated temperature
coefficient, which is usually much higher than in good fixed resistors.

The input attenuator must be adjusted such that a nominal full-scale input 
yields a voltage of 928 mV on the input pins. In the range of ±928 mV is where
the MSP has the best accuracy (at a gain of 1). Otherwise, the setting is not
critical for accuracy. Just make sure not to exceed 928 mV with the range of
interest. In case of doubt, choose an input voltage in excess of the range.

1. Connect an external voltage source of the nominal full-scale input voltage to
     the input;
2. connect a voltmeter directly to the analog inputs of the MSP; then
3. adjust the trimmer for a reading of 928 mV.

### Voltage reference

To calibrate the voltage reference

1. connect a precise (how precise?) external voltage (what value?) to any (which?) input (how can I measure just the external voltage without the input circuit?);
2. measure the applied input voltage,
3. calculate actual reference voltage from theoretical value
4. check plausibility
5. apply

### Input dividers, impedance, & offset

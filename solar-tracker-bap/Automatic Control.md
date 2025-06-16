#### Automatic Control
Automatic control work by using the value get from LDR reading (check [[Weighted Average]]), to control the linear actuator based on that. First we declare the max offset threshold for how big the dead zone (range value when no movement needed) is.

```
const float DEAD_ZONE = 0.05; // 5% deviation is tolerated
const float MAX_TILT = 10.0;  // Max degrees per axis you want to move

if (fabs(normX) < DEAD_ZONE) normX = 0;
if (fabs(normY) < DEAD_ZONE) normY = 0;

// Output the final tilt command (can be used in PWM, servo angle, etc.)
if (normX > 0) {
	tiltX = normX * MAX_TILT;
	motorX.turnLeft(tiltX);
} else if (normX < 0) {
	tiltX = fabs(normX * MAX_TILT);
	motorX.turnRight(tiltX);
} else {
	motorX.stop();
}
// ... Continue for Y axis
```
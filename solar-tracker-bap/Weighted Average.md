#### Weighted Average
The system reading sun value is consist of 6 LDR in each length side of the solar panel. So, implementing the weighting formula for each side:

![[Pasted image 20250615210654.png]]

| Index | Position      | Axis Contribution |
| ----- | ------------- | ----------------- |
| X1    | Top-left      | X: -1, Y: -1      |
| X2    | Top-center    | X: 0, Y: -1       |
| X3    | Top-right     | X: +1, Y: -1      |
| X4    | Bottom-left   | X: -1, Y: +1      |
| X5    | Bottom-center | X: 0, Y: +1       |
| X6    | Bottom-right  | X: +1, Y: +1      |
means the reading of sum total of each `Axis Contribution * analogRead` will result ranging from -1 to 1 since the resolution of each `analogRead` is the same. 

For example, if the solar panel is perfectly flat, the sum of 
``` 
// This will result in 0
for (int i = 0; i < 6; i++) {
    float val = (float)ldr[i];
    sumX += val * dirX[i];
    sumY += val * dirY[i];
    total += val;
}
``` 
will be 0, if the angle is tilted to right side, then the `analogRead` of Top-left and Bottom-left will be bigger and then the result of -X is bigger, and the other way around.
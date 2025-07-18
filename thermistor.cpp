// This functions will calculate the
// temperature in degree centigrade for
// given resistance values
// 3 polynomials of degree 4 are used

// Siemens Matsushita

float getTemperature8016( float resistance){
// Taken from https://arachnoid.com/polysolve/
 float x2, x3, x4;
 x2 = resistance * resistance;
 x3 = resistance * x2;
 x4 = x2 * x2;
 if ( resistance > 53100)
  return (
     -4.4531651972056601e+000 +
    -7.2093688865431456e-004 * resistance +
     5.6956022560950240e-009 * x2 +
    -2.4395812577077518e-014 * x3 +
     4.1270589254171105e-020 * x4
     );
if ( resistance > 9795.0)
  return (
    2.6103137431914661e+001 + 		
    -3.6557765861332070e-003 * resistance +		
     1.1943697688360304e-007 * x2 +	
    -2.0522874639118891e-012 * x3 +	
     1.3650731672152758e-017 * x4	
  );
return (
   8.4048120562685497e+001 +
    -3.9293803104364283e-002 * resistance +
     9.0977907088981083e-006 * x2 +
    -1.0117412967387392e-009 * x3 +	
     4.1306873058676286e-014 * x4
  );
}

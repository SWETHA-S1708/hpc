#include<stdio.h>
#include<omp.h>
double f(double x){
   return x*x;
}

int main(){
   int n, i, thread_count;
   double a, b, h;
   double sum, temp_sum = 0.0;
   printf("Enter Lower Limit(a):");
   scanf("%lf",&a);
   printf("\nEnter Upper Limit(b):");
   scanf("%lf",&b);
   printf("\nEnter number of Trapezoids:");
   scanf("%d",&n);
   printf("\nEnter Thread Count:");
   scanf("%d",&thread_count);
   h = (b - a)/n;
   printf("\nStep 1: h = (b-a)/n = (%.2lf - %.2lf)/%d = %.2lf\n", b, a, n, h);
   sum = (f(a) + f(b)) / 2.0;
   printf("\nStep 2: (f(a)+f(b))/2 = (%.2lf + %.2lf)/2 = %.2lf\n", f(a), f(b), sum);
   printf("\nStep 3: Calculating f(xi) values:\n");
#pragma omp parallel for num_threads(thread_count) reduction(+:temp_sum)
   for(i = 1; i < n; i++){
      double x = a + i * h;
      double fx = f(x);
#pragma omp critical
      {
         printf("i=%d, x=%.2lf, f(x)=%.2lf\n", i, x, fx);
      }

      temp_sum += fx;
   }
   printf("\nStep 4: Sum of intermediate values = %.2lf\n", temp_sum);
   sum += temp_sum;
   printf("\nStep 5: Total sum = %.2lf\n", sum);
   double integral = sum * h;
   printf("\nStep 6: Integral = sum * h = %.2lf * %.2lf = %.2lf\n", sum, h, integral);
   printf("\nEstimated Integral = %.2lf\n", integral);

   return 0;
}
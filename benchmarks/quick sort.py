import time
import board
import digitalio

Pin0 = digitalio.DigitalInOut(board.D0)
Pin0.direction = digitalio.Direction.OUTPUT
Pin2 = digitalio.DigitalInOut(board.D2)
Pin2.direction = digitalio.Direction.OUTPUT
Pin4 = digitalio.DigitalInOut(board.D4)
Pin4.direction = digitalio.Direction.OUTPUT

def partition(arr,low,high): 
    i = ( low-1 )         # index of smaller element 
    pivot = arr[high]     # pivot 
  
    for j in range(low , high): 
  
        # If current element is smaller than or 
        # equal to pivot 
        if   arr[j] <= pivot: 
          
            # increment index of smaller element 
            i = i+1 
            arr[i],arr[j] = arr[j],arr[i] 
  
    arr[i+1],arr[high] = arr[high],arr[i+1] 
    return ( i+1 ) 

def quickSort(arr,low,high): 
    if low < high: 
  
        # pi is partitioning index, arr[p] is now 
        # at right place 
        pi = partition(arr,low,high) 
  
        # Separately sort elements before 
        # partition and after partition 
        quickSort(arr, low, pi-1) 
        quickSort(arr, pi+1, high) 

# 8: (arr[0] == 2995 and arr[len(arr)-1] == 79294)
# 16: (arr[0] == 2995 and arr[len(arr)-1] == 90436)
# 32: (arr[0] == 2995 and arr[len(arr)-1] == 95588)
# 64: (arr[0] == 1565 and arr[len(arr)-1] == 95588)

while True:
    Pin2.value = False
    Pin0.value = True
    arr = []
    for i in range(0, 100):
        arr = [54044,14108,79294,29649,25260,60660,2995,53777,
                49689,9083,16122,90436,4615,40660,25675,58943,
                92904,9900,95588,46120,29390,91323,85363,45738,
                80717,57415,7637,8540,6336,45434,65895,61811,
                
                8959,9139,31027,87662,2484,65550,23260,15616,
                3490,49568,5979,44737,52808,72122,37957,34826,
                21419,73531,94323,52910,84496,71799,50162,1692,
                1565,59279,56864,20141,13893,63942,6055,33424]
                
        quickSort(arr, 0, len(arr)-1)
    Pin0.value = False
    if (arr[0] == 2995 and arr[len(arr-1)] == 79294):
        print("yes")
        Pin4.value = True
        time.sleep(0.2)
        Pin4.value = False
    Pin2.value = True
    time.sleep(1)
# photon-mapping-simple-method
用C++和簡易結合直接及間接光照的方法來實作photon mapping，能夠快速得到caustic的效果
# Method
## First Pass
* 在光源上根據半球往隨機方向發射photon  
* 根據材質做不同停止條件  
  * 只碰到diffuse的photon達到max_depth就停  
  * 有碰到specular的photon只要之後一打到diffuse就停  
* Photon只記錄最後一個打到的點  
* Photon全部存在一個陣列裡  
## Second Pass
* 有加入一次反射且有sample by light  
* 根據第一個打到的位置跟photon位置比較，如果在附近就跟photon顏色相加  
# Result
* Photon Number: 100K
* Photon Depth: 3
* Picture Size: 400 x 400
![image](https://github.com/zz4634266/photon-mapping/blob/main/pm.png?raw=true)

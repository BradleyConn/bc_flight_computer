import matplotlib.pyplot as plt
import math

thrust_n=0
altitude_m=0
acceleration_mps2=0
velocity_mps=0
force_n=0

time=0
time_step = .001
time_end=7
time_end=.01

Kp=0.5
Ki=0
Kd=0

mass_kg=1

g=9.81

tvc_velocity_deg_per_sec = 60/.13 #from the spec sheet
tvc_delay_sec = .005 #check this when the servos are here
tvc_angle_x = [0,0]
tvc_angle_y = [0,0]
motor_thrust_n =[35,35,35,35,35,35,35,0,0,0,0]
for i in range(1000):
    motor_thrust_n.append(0)

class xyz:
    def __init__(self,x,y,z):
        self.x=x
        self.y=y
        self.z=z
    def __str__(self):
        return "x: "+str(self.x)+", y: "+str(self.y)+", z:"+str(self.z)
#global sim frame
sim_linear_acceleration = [xyz(0,0,0), xyz(0,0,0)]
sim_linear_velocity = [xyz(0,0,0), xyz(0,0,0)]
sim_linear_position = [xyz(0,0,0), xyz(0,0,0)]
sim_angular_acceleration = [xyz(0,0,0), xyz(0,0,0)]
sim_angular_velocity = [xyz(0,0,0), xyz(0,0,0)]
#sim_angular_position = [xyz(0,0,0), xyz(math.radians(-10),math.radians(-20),0)]
sim_angular_position = [xyz(10,0,0), xyz(10,0,0)]
#sim_angular_position = [xyz(0,0,0), xyz(0,0,0)]
sim_TVC_angle = [xyz(0,0,0), xyz(0,0,0)]
#rocket sensors
#don't need accel or baro for sim for now
sensor_gyro = [xyz(0,0,0), xyz(0,0,0)]
sensor_acceleration = [xyz(0,0,0), xyz(0,0,0)]
sensor_barometer = [xyz(0,0,0), xyz(0,0,0)]
# This is what the rocket thinks it's orientation is. What the flight computer will calculate
rocket_orientation = [xyz(0,0,0), xyz(0,0,0)]


#MMOI=kg*m^2 = m*g*s^2*d^2/(4*pi^2*d_s)
#s=period of rotation in seconds
#d = distance from CoM to string
#d_s length of string from table

#Moment arm in M
#example values
MMOI = .04
MA = 0.3

#in - force on axis
#out - anglw, angluar velocity, pos, vel, accel

#torque_x = force_x * moment arm (distance to tvc mount)
#torque_y = force_y * moment arm (distance to tvc mount)

#get a desired torque from state space and then calculate servo angle
#apply transfer function and whatever smoothing
#get servo angle and make back into torque for new state


#Display
#Orientation
#TVC Envelope
#Pyrotechnics
#Altitude
#Accelerometers
#Gyroscopes
#RawData
#    ox, oy, ox
#    ax, ay, ax
#    gx, gy, gx
#    altitude
#    temp 
#    volt
#    tvc-y
#    tvc-z

#accelerometer needed for translation, liftoff detect, angle relative to earth

def print_stats():
    print("Altitude (M): " + str(altitude_m))
    print("Acceleration (M/S/S): " + str(acceleration_mps2))
    print("Velocity (M/S): " + str(velocity_mps))
    print("Thrust (N): " + str(thrust_n))

def calculate_angular_acceleration():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    point = xyz(0,0,0)
    # torque = MMOI * angular_Accel
    # torque is newton*meters
    # angular acceleration = newtons*moment arm/ MMOI
    newtons_x = motor_thrust*math.sin(math.radians(tvc_angle_x[-1]))
    print("angular_accel: n = " + str(newtons_x) + " thrust = " + str(motor_thrust) + " tvc angle: " + str(tvc_angle_x[-1]) + " sin " + str( math.sin(math.radians(tvc_angle_x[-1]))))
    point.x = math.degrees(newtons_x * MA / MMOI)
    newtons_y = motor_thrust*math.sin(math.radians(tvc_angle_y[-1]))
    point.y = math.degrees(newtons_y * MA / MMOI)
    sim_angular_acceleration.append(point)
    print ("sim_angular_acceleration: " + str(sim_angular_acceleration[-1]))
def calculate_angular_velocity():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_vel =  sim_angular_velocity[-1]
    angular_accel =  sim_angular_acceleration[-1]
    angular_vel =  xyz(last_vel.x+(angular_accel.x*time_step),
            last_vel.y+(angular_accel.y*time_step),
            last_vel.z+(angular_accel.z*time_step))
    sim_angular_velocity.append(angular_vel)
    print ("sim_angular_velocity: " + str(sim_angular_velocity[-1]))
def calculate_angular_position():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_pos = sim_angular_position[-1]
    vel =  sim_angular_velocity[-1]
    pos =  xyz(last_pos.x+(vel.x*time_step),
            last_pos.y+(vel.y*time_step),
            last_pos.z+(vel.z*time_step))
    sim_angular_position.append(pos)
    print ("sim_angular_position: " + str(sim_angular_position[-1]))

def calculate_linear_acceleration():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_angle = sim_angular_position[-1]
    #for now assume no roll.

    #sin 0 = 0
    #cos 0 = 1
    #Make the axis such that
    #    0 degrees pitch is upright, and 0 degrees yaw is upright
    #    x:90 degrees is yaw to the right, -90 is yaw to the left
    #    y:90 degrees is pitch forward, -90 is pitch backwards
    # yaw then pitch then roll in that order!
    force_x = motor_thrust * math.sin(math.radians(last_angle.x)) * math.cos(last_angle.y)
    force_y = motor_thrust * math.sin(math.radians(last_angle.y))
    force_z = motor_thrust * math.cos(math.radians(last_angle.x)) * math.cos(last_angle.y)
    force_z = force_z - (g * mass_kg)

    #f=ma
    #a=f/m
    accel=xyz(force_x/mass_kg, force_y/mass_kg,force_z/mass_kg)
    sim_linear_acceleration.append(accel)
    print ("sim_linear_acceleration:" + str(sim_linear_acceleration[-1]))

def calculate_linear_velocity():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_vel =  sim_linear_velocity[-1]
    accel =  sim_linear_acceleration[-1]
    vel =  xyz(last_vel.x+(accel.x*time_step),
            last_vel.y+(accel.y*time_step),
            last_vel.z+(accel.z*time_step))
    sim_linear_velocity.append(vel)
    print ("sim_linear_velocity: " + str(sim_linear_velocity[-1]))

def calculate_linear_position():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_pos = sim_linear_position[-1]
    vel =  sim_linear_velocity[-1]
    pos =  xyz(last_pos.x+(vel.x*time_step),
            last_pos.y+(vel.y*time_step),
            last_pos.z+(vel.z*time_step))
    sim_linear_position.append(pos)
    print ("sim_linear_position: " + str(sim_linear_position[-1]))

def tvc_set_angle(x,y):
    tvc_angle_x.append(x)
    tvc_angle_y.append(y)

integral_x = 0
integral_y = 0
def pid(setpoint, axis):
    print("Kp = " + str(Kp))
    #Take the orientation and output torque
    orientation_data = sim_angular_position[-1]
    print(orientation_data)
    ori = 0
    if axis == "x":
        ori = orientation_data.x
    if axis == "y":
        ori = orientation_data.y

    # now do the PID
    error = setpoint - ori
    print("set = " + str(setpoint) + " ori = " + str(ori) + " error = " + str(error))
    proportional = Kp*error

    integral = 0;
    # TODO:add clamping to prevent windup.
    if axis == "x":
        global integral_x
        integral_x = integral_x + (Ki*error*time_step)
        integral = integral_x
    if axis == "y":
        global integral_y
        integral_y = integral_y + (Ki*error*time_step)
        integral = integral_y
    
    derivative = Kd*error/time_step

    output = proportional + integral + derivative
    print("pid = " + str(output))
    return output

def step():
    global time
    global time_step
    time = time + time_step
    print("Step: ")
    motor_thrust = motor_thrust_n[math.floor(time)]
    print("Motor thrust: " + str(motor_thrust))
    calculate_angular_acceleration()
    calculate_angular_velocity()
    calculate_angular_position()
    calculate_linear_acceleration()
    calculate_linear_velocity()
    calculate_linear_position()
    desired_torque_x = pid(0,"x")
    desired_torque_y = pid(0,"y")
#    tvc_angle_x = torque_to_tvc_angle(desired_torque_x)
#    tvc_angle_y = torque_to_tvc_angle(desired_torque_y)
#    tvc_set_angle(tvc_angle_x, tvc_angle_y)
    tvc_set_angle(desired_torque_x, desired_torque_y)


    global altitude_m
    altitude_m = sim_linear_position[-1].z


#Use the global state and derive what the sensors would say
# For now every read is on one sample. But should make this update only after X time.
def read_sensors():
    last_angular_position = sim_angular_position[-1]
    last_last_angular_position = sim_angular_position[-2]
    x_deg_per_sec = last_angular_position.x - last_last_angular_position.x
    y_deg_per_sec = last_angular_position.y - last_last_angular_position.y
    z_deg_per_sec = last_angular_position.z - last_last_angular_position.z
    sensor_gyro.append(xyz(x_deg_per_sec, y_deg_per_sec, z_deg_per_sec))
    #Now track orientation
    last_orientation = rocket_orientation[-1]
    #x_orientation = last_orientation.x + x_deg_per_sec * time_step
    #y_orientation = last_orientation.y + y_deg_per_sec * time_step
    #z_orientation = last_orientation.z + z_deg_per_sec * time_step
    #for now ignore that it's labeled "per sec" and just assume it's per step
    x_orientation = last_orientation.x + x_deg_per_sec
    y_orientation = last_orientation.y + y_deg_per_sec
    z_orientation = last_orientation.z + z_deg_per_sec
    rocket_orientation.append(xyz(x_orientation, y_orientation, z_orientation))

def torque_to_tvc_angle(torque):
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    # avoid divide by 0!
    if motor_thrust < 1:
        return 0
    #torque = newtons * moment arm
    #newtons = thrust*sin(angle)
    #angle = arcsin(newtons/thrust)
    newtons = torque/MA
    a = newtons/motor_thrust
    print("a: " + str(a))
    b = math.asin(a)
    c = math.degrees(b)
    return math.degrees(math.asin(newtons/motor_thrust))


def main():
    global sim_angular_position
    global time
    while altitude_m >= 0:
        read_sensors()
        #pid takes in the rocket orientation (angle) and spits out a torque
        #torque_x = pid(0, "x")
        #torque_y = pid(0, "y")
        #take that torque and calculate a TVC angle
        #motor_thrust = motor_thrust_n[math.floor(i*time_step)]
        #angle_x = torque_to_tvc_angle(torque_x, i)
        #angle_y = torque_to_tvc_angle(torque_y, i)
        #put that into the model
        #sim_angular_position.append(xyz(angle_x, angle_y, 0))
        step()
        print("time: " + str(time) + " seconds")
        if time >= time_end:
            break

    # line 1 points
    t=[]
    acc_x=[]
    acc_y=[]
    acc_z=[]
    vel_x=[]
    vel_y=[]
    vel_z=[]
    pos_x=[]
    pos_y=[]
    pos_z=[]
    angular_acc_x=[]
    angular_acc_y=[]
    angular_acc_z=[]
    angular_vel_x=[]
    angular_vel_y=[]
    angular_vel_z=[]
    angular_pos_x=[]
    angular_pos_y=[]
    angular_pos_z=[]
    for i in range (len(sim_linear_acceleration)):
        t.append(i)
        acc_x.append(sim_linear_acceleration[i].x)
        acc_y.append(sim_linear_acceleration[i].y)
        acc_z.append(sim_linear_acceleration[i].z)
        vel_x.append(sim_linear_velocity[i].x)
        vel_y.append(sim_linear_velocity[i].y)
        vel_z.append(sim_linear_velocity[i].z)
        pos_x.append(sim_linear_position[i].x)
        pos_y.append(sim_linear_position[i].y)
        pos_z.append(sim_linear_position[i].z)
        angular_acc_x.append(sim_angular_acceleration[i].x)
        angular_acc_y.append(sim_angular_acceleration[i].y)
        angular_acc_z.append(sim_angular_acceleration[i].z)
        angular_vel_x.append(sim_angular_velocity[i].x)
        angular_vel_y.append(sim_angular_velocity[i].y)
        angular_vel_z.append(sim_angular_velocity[i].z)
        angular_pos_x.append(sim_angular_position[i].x)
        angular_pos_y.append(sim_angular_position[i].y)
        angular_pos_z.append(sim_angular_position[i].z)


        

    fig, axs = plt.subplots(4,2)
    axs[0][0].plot(t, acc_x, label = "sim_linear_acceleration x")
    axs[0][0].plot(t, acc_y, label = "sim_linear_acceleration y")
    axs[0][0].plot(t, acc_z, label = "sim_linear_acceleration z")
      
    axs[1][0].plot(t, vel_x, label = "sim_linear_velocity x")
    axs[1][0].plot(t, vel_y, label = "sim_linear_velocity y")
    axs[1][0].plot(t, vel_z, label = "sim_linear_velocity z")
      
    axs[2][0].plot(t, pos_x, label = "sim_linear_position x")
    axs[2][0].plot(t, pos_y, label = "sim_linear_position y")
    axs[2][0].plot(t, pos_z, label = "sim_linear_position z")
    
    # naming the y axis
    axs[0][0].title.set_text('Linear Acceleration')
    axs[0][0].set_ylabel('m/s^2')
    axs[0][0].set_xlabel('time')
    axs[0][0].legend("XYZ", loc="upper right")
    axs[1][0].title.set_text('Linear Velocity')
    axs[1][0].set_ylabel('m/s')
    axs[1][0].set_xlabel('time')
    axs[1][0].legend("XYZ", loc="upper right")
    axs[2][0].title.set_text('Linear Position')
    axs[2][0].set_ylabel('m')
    axs[2][0].set_xlabel('time')
    axs[2][0].legend("XYZ", loc="upper right")

    axs[0][1].plot(t, angular_acc_x, label = "sim_angular_acceleration x")
    axs[0][1].plot(t, angular_acc_y, label = "sim_angular_acceleration y")
    axs[0][1].plot(t, angular_acc_z, label = "sim_angular_acceleration z")
      
    axs[1][1].plot(t, angular_vel_x, label = "sim_angular_velocity x")
    axs[1][1].plot(t, angular_vel_y, label = "sim_angular_velocity y")
    axs[1][1].plot(t, angular_vel_z, label = "sim_angular_velocity z")
      
    axs[2][1].plot(t, angular_pos_x, label = "sim_angular_position x")
    axs[2][1].plot(t, angular_pos_y, label = "sim_angular_position y")
    axs[2][1].plot(t, angular_pos_z, label = "sim_angular_position z")

    axs[3][0].plot(t, tvc_angle_x, label = "tvc angle x")
    axs[3][1].plot(t, tvc_angle_y, label = "tvc angle y")
    
    # naming the y axis
    axs[0][1].title.set_text('Angular Acceleration')
    axs[0][1].set_ylabel('deg/s^2')
    axs[0][1].set_xlabel('time')
    axs[0][1].legend("XYZ", loc="upper right")
    axs[1][1].title.set_text('Angular Velocity')
    axs[1][1].set_ylabel('deg/s')
    axs[1][1].set_xlabel('time')
    axs[1][1].legend("XYZ", loc="upper right")
    axs[2][1].title.set_text('Angular Position')
    axs[2][1].set_ylabel('deg')
    axs[2][1].set_xlabel('time')
    axs[2][1].legend("XYZ", loc="upper right")



    fig3D = plt.figure()
    ax3D = plt.axes(projection='3d')
    ax3D.plot(pos_x, pos_y, pos_z, label = "position 3D")
    ax3D.set_xlabel('x')
    ax3D.set_ylabel('y')
    ax3D.set_zlabel('z')
      
    #ang_fig3D = plt.figure()
    #ang_ax3D = plt.axes(projection='3d')
    #ang_ax3D.plot(angular_pos_x, angular_pos_y, angular_pos_z, label = "angular_position 3D")
    #ang_ax3D.set_xlabel('x')
    #ang_ax3D.set_ylabel('y')
    #ang_ax3D.set_zlabel('z')
      
    # show a legend on the plot
    plt.legend()
      
    # function to show the plot
    plt.show()
    print_stats()

main()

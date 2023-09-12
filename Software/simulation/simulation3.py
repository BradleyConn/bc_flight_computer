import matplotlib.pyplot as plt
import math

thrust_n=0
altitude_m=0
acceleration_mps2=0
velocity_mps=0
force_n=0

time=0
time_step = .001
time_end=10

Kp=0.5
Ki=0
Kd=0

mass_kg=1

g=9.81

tvc_velocity_deg_per_sec = 60/.13 #from the spec sheet
tvc_delay_sec = .005 #check this when the servos are here
tvc_set_point = 0
motor_thrust_n =[35,35,35,35,35,35,0,0,0,0,0]
for i in range(1000):
    motor_thrust_n.append(0)

class xy:
    def __init__(self,x,y):
        self.x = x
        self.y=y
    def __str__(self):
        return "x: "+str(self.x)+", y: "+str(self.y)
#global sim frame
sim_linear_acceleration = [xy(0,0), xy(0,0)]
sim_linear_velocity = [xy(0,0), xy(0,0)]
sim_linear_position = [xy(0,0), xy(0,0)]
sim_angular_acceleration = [xy(0,0), xy(0,0)]
sim_angular_velocity = [xy(0,0), xy(0,0)]
sim_angular_position = [xy(0,0), xy(0,0)]
sim_TVC_position = [xy(0,0), xy(0,0)]
#rocket sensors
#don't need accel or baro for sim for now
sensor_gyro = [xy(0,0), xy(0,0)]
sensor_acceleration = [xy(0,0), xy(0,0)]
sensor_barometer = [xy(0,0), xy(0,0)]
# This is what the rocket thinks it's orientation is. What the flight computer will calculate
rocket_orientation = [xy(0,0), xy(0,0)]


#MMOI=kg*m^2 = m*g*s^2*d^2/(4*pi^2*d_s)
#s=period of rotation in seconds
#d = distance from CoM to string
#d_s length of string from table

#Moment arm in M
#example values
MMOI = .04
MA = 0.28

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
    point = xy(0,0)
    if time > .5:
#        point.x = math.radians(90)
        point.y = math.radians(90)
    if time > 1.5:
#        point.x = -(math.radians(90))
        point.y = -(math.radians(90))
    if time > 2.5:
        point.x = 0
        point.y = 0
    sim_angular_acceleration.append(point)
    print ("sim_angular_acceleration: " + str(sim_angular_acceleration[-1]))
def calculate_angular_velocity():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_vel =  sim_angular_velocity[-1]
    angular_accel =  sim_angular_acceleration[-1]
    angular_vel =  xy(last_vel.x+(angular_accel.x*time_step),
            last_vel.y+(angular_accel.y*time_step))
    sim_angular_velocity.append(angular_vel)
    print ("sim_angular_velocity: " + str(sim_angular_velocity[-1]))
def calculate_angular_position():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_pos = sim_angular_position[-1]
    vel =  sim_angular_velocity[-1]
    pos =  xy(last_pos.x+(vel.x*time_step),
            last_pos.y+(vel.y*time_step))
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
    force_x = motor_thrust * math.sin(last_angle.x) * math.cos(last_angle.y)
    force_y = motor_thrust * math.sin(last_angle.x) * math.sin(last_angle.y)
    force_z = motor_thrust * math.cos(last_angle.x)
    force_z = force_z - (g * mass_kg)

    #f=ma
    #a=f/m
    accel=xy(force_x/mass_kg, force_y/mass_kg)
    sim_linear_acceleration.append(accel)
    print ("sim_linear_acceleration:" + str(sim_linear_acceleration[-1]))

def calculate_linear_velocity():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_vel =  sim_linear_velocity[-1]
    accel =  sim_linear_acceleration[-1]
    vel =  xy(last_vel.x+(accel.x*time_step),
            last_vel.y+(accel.y*time_step))
    sim_linear_velocity.append(vel)
    print ("sim_linear_velocity: " + str(sim_linear_velocity[-1]))

def calculate_linear_position():
    global time
    motor_thrust = motor_thrust_n[math.floor(time)]
    last_pos = sim_linear_position[-1]
    vel =  sim_linear_velocity[-1]
    pos =  xy(last_pos.x+(vel.x*time_step),
            last_pos.y+(vel.y*time_step))
    sim_linear_position.append(pos)
    print ("sim_linear_position: " + str(sim_linear_position[-1]))

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

integral = 0
def pid(setpoint, axis):
    print("Kp = " + str(Kp))
    #Take the orientation and output torque
    orientation_data = rocket_orientation[-1]
    ori = 0
    if axis == "x":
        ori = orientation_data.x
    if axis == "y":
        ori = orientation_data.y

    # now do the PID
    error = ori - setpoint
    proportional = Kp*error

    # TODO:add clamping to prevent windup.
    global integral
    integral = integral + (Ki*error*time_step)
    
    derivative = Kd*error/time_step

    output = proportional + integral + derivative
    print(output)
    return output


#Use the global state and derive what the sensors would say
# For now every read is on one sample. But should make this update only after X time.
def read_sensors():
    last_angular_position = sim_angular_position[-1]
    last_last_angular_position = sim_angular_position[-2]
    x_deg_per_sec = last_angular_position.x - last_last_angular_position.x
    y_deg_per_sec = last_angular_position.y - last_last_angular_position.y
    sensor_gyro.append(xy(x_deg_per_sec, y_deg_per_sec))
    #Now track orientation
    last_orientation = rocket_orientation[-1]
    #x_orientation = last_orientation.x + x_deg_per_sec * time_step
    #y_orientation = last_orientation.y + y_deg_per_sec * time_step
    #z_orientation = last_orientation.z + z_deg_per_sec * time_step
    #for now ignore that it's labeled "per sec" and just assume it's per step
    x_orientation = last_orientation.x + x_deg_per_sec
    y_orientation = last_orientation.y + y_deg_per_sec
    rocket_orientation.append(xy(x_orientation, y_orientation))

def torque_to_tvc_angle(torque, i):
    motor_thrust = motor_thrust_n[math.floor(i*time_step)]
    math.asin(1)
    return 5


def main():
    global sim_angular_position
    global time
    while altitude_m >= 0:
        read_sensors()
        #pid takes in the rocket orientation (angle) and spits out a torque
        torque_x = pid(0, "x")
        torque_y = pid(0, "y")
        #take that torque and calculate a TVC angle
        #motor_thrust = motor_thrust_n[math.floor(i*time_step)]
        #angle_x = torque_to_tvc_angle(torque_x, i)
        #angle_y = torque_to_tvc_angle(torque_y, i)
        #put that into the model
        #sim_angular_position.append(xy(angle_x, angle_y, 0))
        step()
        print("time: " + str(time) + " seconds")
        if time >= time_end:
            break

    # line 1 points
    t=[]
    acc_x=[]
    acc_y=[]
    vel_x=[]
    vel_y=[]
    pos_x=[]
    pos_y=[]
    angular_acc_x=[]
    angular_acc_y=[]
    angular_vel_x=[]
    angular_vel_y=[]
    angular_pos_x=[]
    angular_pos_y=[]
    for i in range (len(sim_linear_acceleration)):
        t.append(i)
        acc_x.append(sim_linear_acceleration[i].x)
        acc_y.append(sim_linear_acceleration[i].y)
        vel_x.append(sim_linear_velocity[i].x)
        vel_y.append(sim_linear_velocity[i].y)
        pos_x.append(sim_linear_position[i].x)
        pos_y.append(sim_linear_position[i].y)
        angular_acc_x.append(math.degrees(sim_angular_acceleration[i].x))
        angular_acc_y.append(math.degrees(sim_angular_acceleration[i].y))
        angular_vel_x.append(math.degrees(sim_angular_velocity[i].x))
        angular_vel_y.append(math.degrees(sim_angular_velocity[i].y))
        angular_pos_x.append(math.degrees(sim_angular_position[i].x))
        angular_pos_y.append(math.degrees(sim_angular_position[i].y))


        

    fig, axs = plt.subplots(3,2)
    axs[0][0].plot(t, acc_x, label = "sim_linear_acceleration x")
    axs[0][0].plot(t, acc_y, label = "sim_linear_acceleration y")
      
    axs[1][0].plot(t, vel_x, label = "sim_linear_velocity x")
    axs[1][0].plot(t, vel_y, label = "sim_linear_velocity y")
      
    axs[2][0].plot(t, pos_x, label = "sim_linear_position x")
    axs[2][0].plot(t, pos_y, label = "sim_linear_position y")
    
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
      
    axs[1][1].plot(t, angular_vel_x, label = "sim_angular_velocity x")
    axs[1][1].plot(t, angular_vel_y, label = "sim_angular_velocity y")
      
    axs[2][1].plot(t, angular_pos_x, label = "sim_angular_position x")
    axs[2][1].plot(t, angular_pos_y, label = "sim_angular_position y")
    
    # naming the y axis
    axs[0][1].title.set_text('Angular Acceleration')
    axs[0][1].set_ylabel('deg/s^2')
    axs[0][1].set_xlabel('time')
    axs[0][1].legend("XY", loc="upper right")
    axs[1][1].title.set_text('Angular Velocity')
    axs[1][1].set_ylabel('deg/s')
    axs[1][1].set_xlabel('time')
    axs[1][1].legend("XY", loc="upper right")
    axs[2][1].title.set_text('Angular Position')
    axs[2][1].set_ylabel('deg')
    axs[2][1].set_xlabel('time')
    axs[2][1].legend("XY", loc="upper right")



    #fig3D = plt.figure()
    #ax3D = plt.axes(projection='3d')
    #ax3D.plot(pos_x, pos_y, pos_z, label = "position 3D")
    #ax3D.set_xlabel('x')
    #ax3D.set_ylabel('y')
    #ax3D.set_zlabel('z')
    #  
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

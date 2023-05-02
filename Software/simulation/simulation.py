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

p=0
i=0
d=0

mass_kg=1

g=9.81

motor_thrust_n =[10,11,12,0,0,0,0,0,0,0,0]
for i in range(1000):
    motor_thrust_n.append(0)

class xyz:
    def __init__(self,x,y,z):
        self.x = x
        self.y=y
        self.z=z
    def __str__(self):
        return "x: "+str(self.x)+", y: "+str(self.y)+", z:"+str(self.z)
acceleration = [xyz(0,0,0)]
velocity = [xyz(0,0,0)]
position = [xyz(0,0,0)]
yawPitchRoll = [xyz(.1,0,0)]

def print_stats():
    print("Altitude (M): " + str(altitude_m))
    print("Acceleration (M/S/S): " + str(acceleration_mps2))
    print("Velocity (M/S): " + str(velocity_mps))
    print("Thrust (N): " + str(thrust_n))

def step(i):
    print("Step: ")

    motor_thrust = motor_thrust_n[math.floor(i*time_step)]
    print(str(motor_thrust))
    last_angle = yawPitchRoll[-1]
    #for now assume no roll.
    #this is what wiki claimed. Don't think it's right though.
    #force_x = motor_thrust * math.sin(last_angle.x) * math.cos(last_angle.y)
    #force_y = motor_thrust * math.sin(last_angle.x) * math.sin(last_angle.y)
    #force_z = motor_thrust * math.cos(last_angle.x)
    #force_z = force_z - (g * mass_kg)

    #sin 0 = 0
    #cos 0 = 1
    #Make the axis such that
    #    0 degrees pitch is upright, and 0 degrees yaw is upright
    #    90 degrees is yaw to the right, -90 is yaw to the left
    #    90 degrees is pitch forward, -90 is pitch backwards
    # x is yaw
    force_x = motor_thrust * math.sin(last_angle.x)
    # y is pitch
    force_y = motor_thrust * math.sin(last_angle.y)
    # x is both
    force_z = motor_thrust * (math.cos(last_angle.x) + math.cos(last_angle.y))
    force_z = force_z - (g * mass_kg)


    #f=ma
    #a=f/m
    accel=xyz(force_x/mass_kg, force_y/mass_kg,force_z/mass_kg)
    acceleration.append(accel)
    print ("acceleration:" + str(acceleration[-1]))
    last_vel =  velocity[-1]
    vel =  xyz(last_vel.x+(accel.x*time_step),
            last_vel.y+(accel.y*time_step),
            last_vel.z+(accel.z*time_step))
    velocity.append(vel)
    print ("velocity: " + str(velocity[-1]))
    last_pos = position[-1]
    pos =  xyz(last_pos.x+(vel.x*time_step),
            last_pos.y+(vel.y*time_step),
            last_pos.z+(vel.z*time_step))
    position.append(pos)
    print ("position: " + str(position[-1]))

    global altitude_m
    altitude_m = pos.z

def main():
    global yawPitchRoll
    i=0
    while altitude_m >= 0:
        step(i)
        i=i+1
        print(altitude_m)
        if i == 7:
            yawPitchRoll = [xyz(.25*3.14,0,0)]
        if i == 100:
            yawPitchRoll = [xyz(0,.45,0)]
        if i == 1000:
            yawPitchRoll = [xyz(.45,0,0)]
        if i == 2000:
            yawPitchRoll = [xyz(0,0,0)]
        if i == 2500:
            yawPitchRoll = [xyz(-.45*3.14,-.7,0)]
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
    for i in range (len(acceleration)):
        t.append(i)
        acc_x.append(acceleration[i].x)
        acc_y.append(acceleration[i].y)
        acc_z.append(acceleration[i].z)
        vel_x.append(velocity[i].x)
        vel_y.append(velocity[i].y)
        vel_z.append(velocity[i].z)
        pos_x.append(position[i].x)
        pos_y.append(position[i].y)
        pos_z.append(position[i].z)

        

    fig, axs = plt.subplots(3)
    axs[0].plot(t, acc_x, label = "acceleration x")
    axs[0].plot(t, acc_y, label = "acceleration y")
    axs[0].plot(t, acc_z, label = "acceleration z")
      
    axs[1].plot(t, vel_x, label = "velocity x")
    axs[1].plot(t, vel_y, label = "velocity y")
    axs[1].plot(t, vel_z, label = "velocity z")
      
    axs[2].plot(t, pos_x, label = "position x")
    axs[2].plot(t, pos_y, label = "position y")
    axs[2].plot(t, pos_z, label = "position z")
    
    fig3D = plt.figure()
    ax3D = plt.axes(projection='3d')
    ax3D.plot(pos_x, pos_y, pos_z, label = "position 3D")
    ax3D.set_xlabel('x')
    ax3D.set_ylabel('y')
    ax3D.set_zlabel('z')
      
    # naming the y axis
    axs[0].title.set_text('Acceleration')
    axs[0].set_ylabel('m/s^2')
    axs[0].set_xlabel('time')
    axs[0].legend("XYZ", loc="upper right")
    axs[1].title.set_text('Velocity')
    axs[1].set_ylabel('m/s')
    axs[1].set_xlabel('time')
    axs[1].legend("XYZ", loc="upper right")
    axs[2].title.set_text('Position')
    axs[2].set_ylabel('m')
    axs[2].set_xlabel('time')
    axs[2].legend("XYZ", loc="upper right")
    # giving a title to my graph
      
    # show a legend on the plot
    plt.legend()
      
    # function to show the plot
    plt.show()
    print_stats()

main()

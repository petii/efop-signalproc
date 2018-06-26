copy.fftwpp <- read.csv("fftwppdatacopy.csv")
trns.fftwpp <- read.csv("fftwpptransformation.csv")
copy.vulkan <- read.csv("vulkandatacopy.csv")
trns.vulkan <- read.csv("vulkantransformation.csv")
copy.arless <- read.csv("arrlessdatacopy.csv")
trns.arless <- read.csv("arrlesstransformation.csv")


c.f.means <- colMeans(copy.fftwpp)
c.v.means <- colMeans(copy.vulkan)
t.f.means <- colMeans(trns.fftwpp)
t.v.means <- colMeans(trns.vulkan)
t.v.means <- colMeans(trns.vulkan)/1000

c.a.means <- colMeans(copy.arless)
t.a.means <- colMeans(trns.arless)

f.overall <- c.f.means + t.f.means
v.overall <- c.v.means + t.v.means

#par(mfrow=c(1,1))
t <- c(1:length(c.f.means))
m1 <- lm(c.f.means~t)
m2 <- lm(c.a.means~t)
m3 <- lm(c.v.means~t)


xss <- c(10:50)
xs <- c(10:200)
text <- c('FFTW++ Array','FFTW++ [ ]','Vulkan')
plot(xs,c.f.means,type='l',
     xlab = 'Sample size (x256)', 
     ylab = 'Time (nanosecs)', 
     main = 'Copy')
lines(xs,c.a.means,col=3)
lines(xss,c.v.means,col=2)
legend(x = "topleft",legend = text, col=c(1,3,2), lty=c(1,1,1))

#plot(m1$fitted.value

xss <- c(10:50)
xs <- c(10:200)
text <- c('FFTW++ Array','FFTW++ [ ]','Vulkan (microseconds)')
plot(xs,t.f.means,type='l',
     xlab = 'Sample size (x256)', 
     ylab = 'Time (nanosecs)', 
     main = 'Transform')
lines(xs,t.a.means,col=3)
lines(xss,t.v.means,col=2)
legend(x="bottomright",legend = text, col=c(1,3,2), lty=c(1,1,1))

#plot(m1$fitted.values, col=6, type='l')
#lines(m2$fitted.values,col=4)
#lines(m3$fitted.values, col=5)

plot(c.v.means,type='l')
lines(c.a.means)
lines(c.f.means)

plot(t.f.means,type='l')
lines(t.a.means)
lines(t.v.means)

diff <- abs(t.f.means - t.a.means)

lines(diff)

plot(t.v.means,type='l')
lines(t.f.means)

plot(f.overall, type = 'l')
lines(v.overall)

plot(v.overall, type = 'l')
lines(f.overall)

model

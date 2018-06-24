copy.fftwpp <- read.csv("fftwppdatacopy.csv")
trns.fftwpp <- read.csv("fftwpptransformation.csv")
copy.vulkan <- read.csv("vulkandatacopy.csv")
trns.vulkan <- read.csv("vulkantransformation.csv")

c.f.means <- colMeans(copy.fftwpp)
c.v.means <- colMeans(copy.vulkan)
t.f.means <- colMeans(trns.fftwpp)
t.v.means <- colMeans(trns.vulkan)

#par(mfrow=c(1,1))

plot(c.f.means,type='l')
lines(c.v.means)

plot(t.f.means,type='l')
lines(t.v.means)


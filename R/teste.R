

#sudo apt-get install r-base r-base-core

# install.packages("languageserver")

getwd()

setwd("/home/muriel/github/TspHeuristics/R/")

tabela <- read.table("data100r.csv",header=TRUE,sep=",")



boxplot(tabela,

main="Variação das distancias em relação ao Alpha",
xlab="Alpha Grasp",
ylab="Distancia",
las=1)



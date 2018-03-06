FROM node

RUN apt-get update && apt-get install curl nodejs npm -y

RUN mkdir -p /home/nodejs/app
COPY ./init.sh /home/nodejs/app/init.sh
RUN chmod u+x /home/nodejs/app/init.sh
COPY ./index.js /home/nodejs/app/index.js
COPY ./package.json /home/nodejs/app/package.json
WORKDIR /home/nodejs/app
RUN npm install

CMD ./init.sh
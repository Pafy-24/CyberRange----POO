FROM node:18-alpine

WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci
COPY . .

RUN mkdir -p public/uploads && chmod 755 public/uploads

RUN npm run build -- --no-lint

EXPOSE 3000

CMD ["npm", "start"] 
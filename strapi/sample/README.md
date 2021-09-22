# Sample strapi app with MySQL

## Dev

<dl>
<dt><code>yarn develop</code>
<dd>Start Strapi in watch mode.
<dt><code>yarn start</code>
<dd>Start Strapi without watch mode.
<dt><code>yarn build</code>
<dd>Build Strapi admin panel.
<dt><code>yarn strapi</code>
<dd>Display all available commands.
</dl>

### Database

Create a mysql container:

```bash
docker run \
  --name mysql8 \
  -e MYSQL_ROOT_PASSWORD=secret \
  -e MYSQL_DATABASE=sample \
  -p 3306:3306 \
  -d \
  mysql:8.0.18 --character-set-server=utf8mb4 --collation-server=utf8mb4_bin
```

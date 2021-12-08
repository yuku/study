module.exports = ({ env }) => ({
  auth: {
    secret: env('ADMIN_JWT_SECRET', '1331ca03f8625286ec927289ae3877ef'),
  },
});

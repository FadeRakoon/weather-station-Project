import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  { 
    path: '/', 
    name: 'Dashboard', 
    component: () => import('@/views/Live.vue'), 
    meta: { transition: 'fade' } 
  }, 
  { 
    path: '/analysis', 
    name: 'Analysis', 
    component: () => import('@/views/Analysis.vue'), 
    meta: { transition: 'fade' } 
  },
];

const router = createRouter({
  history: createWebHistory(process.env.BASE_URL),
  routes,
})

export default router
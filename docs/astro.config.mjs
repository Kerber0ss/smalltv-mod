import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
  integrations: [
    starlight({
      title: 'smalltv-mod',
      description:
        'Open-source ESP8266 firmware for the GeekMagic SmallTV: air-alert Radar, usage metrics, and Home Assistant screens.',
      logo: {
        src: './src/assets/logo.svg',
        replacesTitle: false,
      },
      social: [
        {
          icon: 'github',
          label: 'GitHub',
          href: 'https://github.com/giovi321/smalltv-mod',
        },
      ],
      editLink: {
        baseUrl: 'https://github.com/giovi321/smalltv-mod/edit/main/docs/',
      },
      sidebar: [
        { label: 'Home', link: '/' },
        {
          label: 'Getting started',
          items: [
            { label: 'Hardware', link: '/getting-started/hardware/' },
            { label: 'Flashing', link: '/getting-started/flashing/' },
            { label: 'First-time setup', link: '/getting-started/setup/' },
          ],
        },
        {
          label: 'User manual',
          items: [
            { label: 'Quick start', link: '/manual/quick-start/' },
            { label: 'Everyday use', link: '/manual/everyday/' },
            { label: 'Settings explained', link: '/manual/settings/' },
            { label: 'Troubleshooting', link: '/manual/troubleshooting/' },
          ],
        },
        {
          label: 'Features',
          items: [
            { label: 'Claude usage meter', link: '/features/usage/' },
            { label: 'Air-alert Radar', link: '/features/radar/' },
            { label: 'Notifications', link: '/features/notify/' },
            { label: 'Home Assistant screens', link: '/features/ha/' },
          ],
        },
        {
          label: 'Reference',
          items: [
            { label: 'Which release file to download', link: '/reference/release-assets/' },
            { label: 'Building from source', link: '/reference/building/' },
            { label: 'Recovery and credits', link: '/reference/recovery/' },
          ],
        },
      ],
    }),
  ],
});
